#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that calculates AI
 */
class AiSystem : public RaccoonEcs::System
{
public:
	AiSystem(WorldHolder& worldHolder) noexcept;

	~AiSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "AiSystem"; }

private:
	WorldHolder& mWorldHolder;
};
