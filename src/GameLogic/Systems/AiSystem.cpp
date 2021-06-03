#include "Base/precomp.h"

#include "GameLogic/Systems/AiSystem.h"

#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/PathBlockingGeometryComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"
#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/AI/NavMeshGenerator.h"
#include "Utils/AI/PathFinding.h"

AiSystem::AiSystem(
		RaccoonEcs::ComponentAdder<NavMeshComponent>&& navMeshDataFilter,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>&& collisionDataFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		NavDataReader&& navDataFilter,
		RaccoonEcs::ComponentFilter<DebugDrawComponent>&& debugDrawFilter,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const PathBlockingGeometryComponent>&& pathBlockingGeometryFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData
	) noexcept
	: mNavMeshDataFilter(std::move(navMeshDataFilter))
	, mCollisionDataFilter(std::move(collisionDataFilter))
	, mTransformFilter(std::move(transformFilter))
	, mNavDataFilter(std::move(navDataFilter))
	, mDebugDrawFilter(std::move(debugDrawFilter))
	, mTrackedFilter(std::move(trackedFilter))
	, mPathBlockingGeometryFilter(std::move(pathBlockingGeometryFilter))
	, mWorldHolder(worldHolder)
	, mTime(timeData)
{
}

void AiSystem::update()
{
	World& world = mWorldHolder.getWorld();
	const GameplayTimestamp timestampNow = mTime.currentTimestamp;

	NavMeshComponent* navMeshComponent = mNavMeshDataFilter.getOrAddComponent(world.getWorldComponents());

	auto [pathBlockingGeometry] = mPathBlockingGeometryFilter.getComponents(world.getWorldComponents());

	if (pathBlockingGeometry == nullptr)
	{
		return;
	}

	TupleVector<const CollisionComponent*, const TransformComponent*> collisions;
	world.getSpatialData().getAllCellManagers().getComponents(mCollisionDataFilter, collisions);

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
		NavMesh& navMesh = navMeshComponent->getNavMeshRef();
		NavMeshGenerator::GenerateNavMeshGeometry(navMesh.geometry, pathBlockingGeometry->getPolygons(), Vector2D(-5000.0f, -5000.0f), Vector2D(10000.0f, 10000.0f));
		NavMeshGenerator::LinkNavMesh(navMesh.links, navMesh.geometry);
		NavMeshGenerator::BuildSpatialHash(navMesh.spatialHash, navMesh.geometry, NavMeshGenerator::HashGenerationType::Fast);
		navMeshComponent->setUpdateTimestamp(timestampNow);
	}

	std::optional<std::pair<AsyncEntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("ControlledEntity"));

	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform] = playerEntity->first.getComponents(mTransformFilter);
	if (playerTransform == nullptr)
	{
		return;
	}

	GameplayTimestamp navmeshUpdateTimestamp = navMeshComponent->getUpdateTimestamp();

	auto [debugDraw] = mDebugDrawFilter.getComponents(mWorldHolder.getGameData().getGameComponents());

	Vector2D targetLocation = playerTransform->getLocation();

	const NavMesh& navMesh = navMeshComponent->getNavMesh();

	world.getSpatialData().getAllCellManagers().forEachComponentSet(
		mNavDataFilter,
		[targetLocation, &navMesh, timestampNow, navmeshUpdateTimestamp, debugDraw]
			(AiControllerComponent* aiController, const TransformComponent* transform, MovementComponent* movement, CharacterStateComponent* characterState)
	{
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
