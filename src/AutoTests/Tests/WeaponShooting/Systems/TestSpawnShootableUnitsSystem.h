#pragma once

#include <unordered_map>

#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"

#include "GameData/EcsDefinitions.h"
#include "GameData/Core/Vector2D.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


class TestSpawnShootableUnitsSystem : public RaccoonEcs::System
{
public:
	explicit TestSpawnShootableUnitsSystem(
		RaccoonEcs::ComponentAdder<TransformComponent>&& transformAdder,
		RaccoonEcs::ComponentAdder<CollisionComponent>&& collisionAdder,
		RaccoonEcs::ComponentAdder<HealthComponent>&& healthAdder,
		RaccoonEcs::ComponentAdder<SpriteCreatorComponent>&& spriteCreatorAdder,
		RaccoonEcs::EntityAdder&& entityAdder,
		WorldHolder& worldHolder) noexcept;

	void update() override;
	std::string getName() const override { return "TestSpawnShootableUnitsSystem"; }

private:
	void spawnUnit(AsyncEntityManager& entityManager, Vector2D pos);
	void spawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, class SpatialWorldData& spatialData);
	void spawnUnits(class SpatialWorldData& spatialData, int count, Vector2D pos);

private:
	RaccoonEcs::ComponentAdder<TransformComponent> mTransformAdder;
	RaccoonEcs::ComponentAdder<CollisionComponent> mCollisionAdder;
	RaccoonEcs::ComponentAdder<HealthComponent> mHealthAdder;
	RaccoonEcs::ComponentAdder<SpriteCreatorComponent> mSpriteCreatorAdder;
	RaccoonEcs::EntityAdder mEntityAdder;
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int JitterRand = 500;
	static inline const float JitterMax = 30.0f;
	static inline const float HalfJitterMax = JitterMax / 2.0f;
	static inline const float JitterDivider = JitterRand / JitterMax;
	static inline const float Distance = 50.0f;
};
