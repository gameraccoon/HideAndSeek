#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestUnitsCountControlSystem.h"

#include "Base/Random/Random.h"

#include "GameData/World.h"
#include "GameData/Spatial/SpatialWorldData.h"
#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


TestUnitsCountControlSystem::TestUnitsCountControlSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void TestUnitsCountControlSystem::SpawnUnit(EntityManager& entityManager, const Vector2D& pos)
{
	const Entity entity = entityManager.addEntity();
	{
		TransformComponent* transform = entityManager.addComponent<TransformComponent>(entity);
		transform->setLocation(pos);
	}
	{
		MovementComponent* movement = entityManager.addComponent<MovementComponent>(entity);
		movement->setOriginalSpeed(2.0f);
	}
	{
		SpriteCreatorComponent* sprite = entityManager.addComponent<SpriteCreatorComponent>(entity);
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(20.0f, 20.0f);
		spriteDesc.path = RelativeResourcePath("resources/textures/hero.png");
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = entityManager.addComponent<CollisionComponent>(entity);
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(10.0f);
	}
	entityManager.addComponent<AiControllerComponent>(entity);
	entityManager.addComponent<CharacterStateComponent>(entity);
}

void TestUnitsCountControlSystem::SpawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData)
{
	const Vector2D jitter = Vector2D(static_cast<float>(Random::gGlobalGenerator() % jitterRand) / jitterDivider - halfJitterMax, (static_cast<float>(Random::gGlobalGenerator() % jitterRand) / jitterDivider - halfJitterMax));
	const Vector2D newPos = centerShifted + pos + jitter;
	const CellPos cellPos = SpatialWorldData::GetCellForPos(newPos);
	SpawnUnit(spatialData.getOrCreateCell(cellPos).getEntityManager(), newPos);
}

void TestUnitsCountControlSystem::SpawnUnits(SpatialWorldData& spatialData, const int count, const Vector2D& pos)
{
	const int n = static_cast<int>(std::sqrt(count));
	const int m = count / n;

	const Vector2D centerShifted = pos - Vector2D(static_cast<float>(n - 1), m - ((count == m * n) ? 1.0f : 0.0f)) * (distance * 0.5f);

	for (int j = 0; j < m; ++j)
	{
		for (int i = 0; i < n; ++i)
		{
			SpawnJitteredUnit(Vector2D(i * distance, j * distance), centerShifted, spatialData);
		}
	}

	const float yPos = static_cast<float>(m) * distance;
	const int unitsLeft = count - m * n;
	for (int i = 0; i < unitsLeft; ++i)
	{
		SpawnJitteredUnit(Vector2D(i * distance, yPos), centerShifted, spatialData);
	}
}

void TestUnitsCountControlSystem::update()
{
	World& world = mWorldHolder.getWorld();

	if (mTicksPassed == 5)
	{
		SpawnUnits(world.getSpatialData(), 500, Vector2D(-400.0f, 0.0f));
	}

	++mTicksPassed;
}
