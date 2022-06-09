#pragma once

#include <raccoon-ecs/systems_manager.h>
#include <raccoon-ecs/component_factory.h>

#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/Application/ArgumentsParser.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/BaseTestCase.h"
#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

class WeaponShootingTestCase : public BaseTestCase
{
public:
	using BaseTestCase::BaseTestCase;

protected:
	void initTestCase(const ArgumentsParser& arguments) override;
};
