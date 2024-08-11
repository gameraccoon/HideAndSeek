#pragma once

#include <raccoon-ecs/component_factory.h>
#include <raccoon-ecs/utils/systems_manager.h>

#include "GameData/GameData.h"
#include "GameData/World.h"

#include "EngineUtils/Application/ArgumentsParser.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/BaseTestCase.h"
#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

class WeaponShootingTestCase final : public BaseTestCase
{
public:
	using BaseTestCase::BaseTestCase;

protected:
	void initTestCase(const ArgumentsParser& arguments) override;
};
