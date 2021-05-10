#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

class TestDestroyedEntitiesRegistrationSystem : public RaccoonEcs::System
{
public:
	TestDestroyedEntitiesRegistrationSystem(WorldHolder& worldHolder, DestroyedEntitiesTestCheck& testCheck) noexcept;

	void update() override;
	std::string getName() override { return "TestSpawnShootableUnitsSystem"; }

private:
	WorldHolder& mWorldHolder;
	DestroyedEntitiesTestCheck& mTestCheck;
};
