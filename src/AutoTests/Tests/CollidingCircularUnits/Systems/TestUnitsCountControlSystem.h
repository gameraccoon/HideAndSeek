#pragma once

#include <raccoon-ecs/utils/system.h>

#include "GameData/EcsDefinitions.h"

class WorldHolder;
class SpatialWorldData;
struct Vector2D;

class TestUnitsCountControlSystem final : public RaccoonEcs::System
{
public:
	explicit TestUnitsCountControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	static void SpawnUnit(EntityManager& entityManager, const Vector2D& pos);
	static void SpawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	static void SpawnUnits(SpatialWorldData& spatialData, int count, const Vector2D& pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static constexpr int jitterRand = 500;
	static constexpr float jitterMax = 30.0f;
	static constexpr float halfJitterMax = jitterMax / 2.0f;
	static constexpr float jitterDivider = jitterRand / jitterMax;
	static constexpr float distance = 50.0f;
};
