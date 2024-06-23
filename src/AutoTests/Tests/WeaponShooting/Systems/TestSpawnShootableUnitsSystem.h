#pragma once

#include <raccoon-ecs/utils/system.h>

#include "GameData/EcsDefinitions.h"

class SpatialWorldData;
class WorldHolder;
struct Vector2D;

class TestSpawnShootableUnitsSystem final : public RaccoonEcs::System
{
public:
	explicit TestSpawnShootableUnitsSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	static void spawnUnit(EntityManager& entityManager, const Vector2D& pos);
	static void spawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	static void spawnUnits(SpatialWorldData& spatialData, int count, const Vector2D& pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static constexpr int JitterRand = 500;
	static constexpr float JitterMax = 30.0f;
	static constexpr float HalfJitterMax = JitterMax / 2.0f;
	static constexpr float JitterDivider = JitterRand / JitterMax;
	static constexpr float Distance = 50.0f;
};
