#pragma once

#include <raccoon-ecs/utils/system.h>

#include "GameData/EcsDefinitions.h"

class WorldHolder;
class SpatialWorldData;
struct Vector2D;

class TestUnitsCountControlSystem : public RaccoonEcs::System
{
public:
	TestUnitsCountControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	void SpawnUnit(EntityManager& entityManager, const Vector2D& pos);
	void SpawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	void SpawnUnits(SpatialWorldData& spatialData, int count, const Vector2D& pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int jitterRand = 500;
	static inline const float jitterMax = 30.0f;
	static inline const float halfJitterMax = jitterMax / 2.0f;
	static inline const float jitterDivider = jitterRand / jitterMax;
	static inline const float distance = 50.0f;
};
