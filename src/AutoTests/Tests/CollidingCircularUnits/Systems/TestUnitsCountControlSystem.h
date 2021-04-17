#pragma once

#include <unordered_map>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

class TestUnitsCountControlSystem : public System
{
public:
	explicit TestUnitsCountControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;
	std::string getName() override { return "TestUnitsCountControlSystem"; }

private:
	static void SpawnUnit(EntityManager& entityManager, Vector2D pos);
	static void SpawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, SpatialWorldData& spatialData);
	static void SpawnUnits(SpatialWorldData& spatialData, int count, Vector2D pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int jitterRand = 500;
	static inline const float jitterMax = 30.0f;
	static inline const float halfJitterMax = jitterMax / 2.0f;
	static inline const float jitterDivider = jitterRand / jitterMax;
	static inline const float distance = 50.0f;
};
