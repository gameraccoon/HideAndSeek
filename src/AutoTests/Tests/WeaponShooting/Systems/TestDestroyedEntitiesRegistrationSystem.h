#pragma once

#include <unordered_map>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

class TestDestroyedEntitiesRegistrationSystem : public Ecs::System
{
public:
	TestDestroyedEntitiesRegistrationSystem(WorldHolder& worldHolder, DestroyedEntitiesTestCheck& testCheck) noexcept;

	void update() override;
	std::string getName() override { return "TestSpawnShootableUnitsSystem"; }

private:
	WorldHolder& mWorldHolder;
	DestroyedEntitiesTestCheck& mTestCheck;
};
