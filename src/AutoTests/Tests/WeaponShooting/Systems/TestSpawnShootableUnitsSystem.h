#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameData/EcsDefinitions.h"
#include "GameData/Geometry/Vector2D.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


class TestSpawnShootableUnitsSystem : public RaccoonEcs::System
{
public:
	explicit TestSpawnShootableUnitsSystem(WorldHolder& worldHolder) noexcept;

	void update() override;
	static std::string GetSystemId() { return "TestSpawnShootableUnitsSystem"; }

private:
	void spawnUnit(EntityManager& entityManager, Vector2D pos);
	void spawnJitteredUnit(const Vector2D& pos, const Vector2D& centerShifted, class SpatialWorldData& spatialData);
	void spawnUnits(class SpatialWorldData& spatialData, int count, Vector2D pos);

private:
	WorldHolder& mWorldHolder;
	int mTicksPassed = 0;

	static inline const int JitterRand = 500;
	static inline const float JitterMax = 30.0f;
	static inline const float HalfJitterMax = JitterMax / 2.0f;
	static inline const float JitterDivider = JitterRand / JitterMax;
	static inline const float Distance = 50.0f;
};
