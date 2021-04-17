#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestSpawnShootableUnitsSystem.h"

#include "Base/Random/Random.h"

#include <sdl/SDL_keycode.h>

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"

#include "GameData/World.h"
#include "GameData/Spatial/SpatialWorldData.h"

TestSpawnShootableUnitsSystem::TestSpawnShootableUnitsSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void TestSpawnShootableUnitsSystem::spawnUnit(EntityManager& entityManager, Vector2D pos)
{
	Entity entity = entityManager.addEntity();
	{
		TransformComponent* transform = entityManager.addComponent<TransformComponent>(entity);
		transform->setLocation(pos);
	}
	{
		SpriteCreatorComponent* sprite = entityManager.addComponent<SpriteCreatorComponent>(entity);
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(20.0f, 20.0f);
		spriteDesc.path = "resources/textures/hero.png";
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = entityManager.addComponent<CollisionComponent>(entity);
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(10.0f);
	}
	{
		HealthComponent* health = entityManager.addComponent<HealthComponent>(entity);
		health->setHealthValue(100.0f);
	}
}

void TestSpawnShootableUnitsSystem::spawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData)
{
	Vector2D jitter = Vector2D(static_cast<float>(Random::gGlobalGenerator() % JitterRand) / JitterDivider - HalfJitterMax, (static_cast<float>(Random::gGlobalGenerator() % JitterRand) / JitterDivider - HalfJitterMax));
	Vector2D newPos = centerShifted + pos + jitter;
	CellPos cellPos = SpatialWorldData::GetCellForPos(newPos);
	spawnUnit(spatialData.getOrCreateCell(cellPos).getEntityManager(), newPos);
}

void TestSpawnShootableUnitsSystem::spawnUnits(SpatialWorldData& spatialData, int count, Vector2D pos)
{
	int n = static_cast<int>(std::sqrt(count));
	int m = count / n;

	Vector2D centerShifted = pos - Vector2D(static_cast<float>(n - 1), static_cast<float>(m) - ((count == m * n) ? 1.0f : 0.0f)) * (Distance * 0.5f);

	for (int j = 0; j < m; ++j)
	{
		for (int i = 0; i < n; ++i)
		{
			spawnJitteredUnit(Vector2D(static_cast<float>(i) * Distance, static_cast<float>(j) * Distance), centerShifted, spatialData);
		}
	}

	float yPos = static_cast<float>(m) * Distance;
	int unitsLeft = count - m * n;
	for (int i = 0; i < unitsLeft; ++i)
	{
		spawnJitteredUnit(Vector2D(static_cast<float>(i) * Distance, yPos), centerShifted, spatialData);
	}
}

void TestSpawnShootableUnitsSystem::update()
{
	World& world = mWorldHolder.getWorld();

	if (mTicksPassed == 5)
	{
		spawnUnits(world.getSpatialData(), 100, Vector2D(-100.0f, 0.0f));
	}

	++mTicksPassed;
}
