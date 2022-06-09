#include "Base/precomp.h"

#include "GameLogic/Systems/AiSystem.h"

#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/PathBlockingGeometryComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "Utils/AI/NavMeshGenerator.h"
#include "Utils/AI/PathFinding.h"

AiSystem::AiSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void AiSystem::update()
{
	SCOPED_PROFILER("AiSystem::update");
	World& world = mWorldHolder.getWorld();

	const auto [time] = world.getWorldComponents().getComponents<TimeComponent>();
	const GameplayTimestamp timestampNow = time->getValue().lastFixedUpdateTimestamp;

	NavMeshComponent* navMeshComponent = world.getWorldComponents().getOrAddComponent<NavMeshComponent>();

	auto [pathBlockingGeometry] = world.getWorldComponents().getComponents<PathBlockingGeometryComponent>();

	if (pathBlockingGeometry == nullptr)
	{
		return;
	}

	TupleVector<const CollisionComponent*, const TransformComponent*> collisions;
	world.getSpatialData().getAllCellManagers().getComponents<const CollisionComponent, const TransformComponent>(collisions);

	bool needUpdate = !navMeshComponent->getNavMesh().geometry.isCalculated;
	if (!needUpdate)
	{
		needUpdate = std::any_of(std::begin(collisions), std::end(collisions), [lastUpdateTimestamp = navMeshComponent->getUpdateTimestamp()](const std::tuple<const CollisionComponent*, const TransformComponent*>& set)
		{
			GameplayTimestamp objectUpdateTimestamp = std::get<1>(set)->getUpdateTimestamp();
			return objectUpdateTimestamp > lastUpdateTimestamp && std::get<0>(set)->getGeometry().type == HullType::Angular;
		});
	}

	if (needUpdate)
	{
		SCOPED_PROFILER("AiSystem::update navmesh");
		NavMesh& navMesh = navMeshComponent->getNavMeshRef();
		NavMeshGenerator::GenerateNavMeshGeometry(navMesh.geometry, pathBlockingGeometry->getPolygons(), Vector2D(-5000.0f, -5000.0f), Vector2D(10000.0f, 10000.0f));
		NavMeshGenerator::LinkNavMesh(navMesh.links, navMesh.geometry);
		NavMeshGenerator::BuildSpatialHash(navMesh.spatialHash, navMesh.geometry, NavMeshGenerator::HashGenerationType::Fast);
		navMeshComponent->setUpdateTimestamp(timestampNow);
	}

	std::optional<std::pair<EntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));

	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform] = playerEntity->first.getComponents<TransformComponent>();
	if (playerTransform == nullptr)
	{
		return;
	}

	GameplayTimestamp navmeshUpdateTimestamp = navMeshComponent->getUpdateTimestamp();

	auto [debugDraw] = mWorldHolder.getGameData().getGameComponents().getComponents<DebugDrawComponent>();

	Vector2D targetLocation = playerTransform->getLocation();

	const NavMesh& navMesh = navMeshComponent->getNavMesh();

	world.getSpatialData().getAllCellManagers().forEachComponentSet<AiControllerComponent, const TransformComponent, MovementComponent, CharacterStateComponent>(
		[targetLocation, &navMesh, timestampNow, navmeshUpdateTimestamp, debugDraw]
			(AiControllerComponent* aiController, const TransformComponent* transform, MovementComponent* movement, CharacterStateComponent* characterState)
	{
		SCOPED_PROFILER("AiSystem::update update one character");
		Vector2D currentLocation = transform->getLocation();

		TravelPath& pathData = aiController->getPathRef();
		std::vector<Vector2D> &path = pathData.smoothPath;
		if (path.empty() || pathData.targetPos != targetLocation || pathData.updateTimestamp < navmeshUpdateTimestamp)
		{
			PathFinding::FindPath(path, navMesh, currentLocation, targetLocation);

			for (size_t i = 1; i < path.size(); ++i)
			{
				debugDraw->getWorldLineSegmentsRef().emplace_back(path[i - 1], path[i], timestampNow.getIncreasedByFloatTime(10.0f));
			}

			characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToMove, path.size() > 1);
			pathData.targetPos = targetLocation;
			pathData.updateTimestamp = timestampNow;
		}

		if (!path.empty())
		{
			if ((path[0] - currentLocation).qSize() < 20.0f)
			{
				path.erase(path.begin());
			}
		}

		if (!path.empty())
		{
			Vector2D diff = path[0] - currentLocation;
			movement->setMoveDirection(diff);
		}
		else
		{
			movement->setMoveDirection(ZERO_VECTOR);
		}
	});
}
