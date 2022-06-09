#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

class TestCircularUnitsSystem : public RaccoonEcs::System
{
public:
	TestCircularUnitsSystem(WorldHolder& worldHolder) noexcept;
	~TestCircularUnitsSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "TestCircularUnitsSystem"; }

private:
	WorldHolder& mWorldHolder;
};
