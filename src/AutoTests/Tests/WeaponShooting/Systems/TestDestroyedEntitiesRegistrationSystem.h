#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

class DeathComponent;

class TestDestroyedEntitiesRegistrationSystem : public RaccoonEcs::System
{
public:
	TestDestroyedEntitiesRegistrationSystem(
		RaccoonEcs::ComponentFilter<const DeathComponent>&& deathFilter,
		WorldHolder& worldHolder,
		DestroyedEntitiesTestCheck& testCheck) noexcept;

	void update() override;
	std::string getName() const override { return "TestSpawnShootableUnitsSystem"; }

private:
	RaccoonEcs::ComponentFilter<const DeathComponent> mDeathFilter;
	WorldHolder& mWorldHolder;
	DestroyedEntitiesTestCheck& mTestCheck;
};
