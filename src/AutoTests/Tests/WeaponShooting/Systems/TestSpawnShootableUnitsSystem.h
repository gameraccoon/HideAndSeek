#pragma once

#include <raccoon-ecs/system.h>

#include "GameData/EcsDefinitions.h"

class SpatialWorldData;
class WorldHolder;
struct Vector2D;

class TestSpawnShootableUnitsSystem : public RaccoonEcs::System
{
public:
	TestSpawnShootableUnitsSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	void spawnUnit(EntityManager& entityManager, const Vector2D& pos);
	void spawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	void spawnUnits(SpatialWorldData& spatialData, int count, const Vector2D& pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int JitterRand = 500;
	static inline const float JitterMax = 30.0f;
	static inline const float HalfJitterMax = JitterMax / 2.0f;
	static inline const float JitterDivider = JitterRand / JitterMax;
	static inline const float Distance = 50.0f;
};
