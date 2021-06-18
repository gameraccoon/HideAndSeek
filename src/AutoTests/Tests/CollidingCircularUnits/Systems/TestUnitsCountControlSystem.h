#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

class TestUnitsCountControlSystem : public RaccoonEcs::System
{
public:
	TestUnitsCountControlSystem(
		RaccoonEcs::EntityAdder&& entityAdder,
		RaccoonEcs::ComponentAdder<TransformComponent>&& transformAdder,
		RaccoonEcs::ComponentAdder<MovementComponent>&& movementAdder,
		RaccoonEcs::ComponentAdder<SpriteCreatorComponent>&& spriteCreatorAdder,
		RaccoonEcs::ComponentAdder<CollisionComponent>&& collisionAdder,
		RaccoonEcs::ComponentAdder<AiControllerComponent>&& aiControllerAdder,
		RaccoonEcs::ComponentAdder<CharacterStateComponent>&& characterStateAdder,
		WorldHolder& worldHolder) noexcept;

	void update() override;
	static std::string GetSystemId() { return "TestUnitsCountControlSystem"; }

private:
	void SpawnUnit(AsyncEntityManager& entityManager, Vector2D pos);
	void SpawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	void SpawnUnits(SpatialWorldData& spatialData, int count, Vector2D pos);

private:
	RaccoonEcs::EntityAdder mEntityAdder;
	RaccoonEcs::ComponentAdder<TransformComponent> mTransformAdder;
	RaccoonEcs::ComponentAdder<MovementComponent> mMovementAdder;
	RaccoonEcs::ComponentAdder<SpriteCreatorComponent> mSpriteCreatorAdder;
	RaccoonEcs::ComponentAdder<CollisionComponent> mCollisionAdder;
	RaccoonEcs::ComponentAdder<AiControllerComponent> mAiControllerAdder;
	RaccoonEcs::ComponentAdder<CharacterStateComponent> mCharacterStateAdder;
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int jitterRand = 500;
	static inline const float jitterMax = 30.0f;
	static inline const float halfJitterMax = jitterMax / 2.0f;
	static inline const float jitterDivider = jitterRand / jitterMax;
	static inline const float distance = 50.0f;
};
