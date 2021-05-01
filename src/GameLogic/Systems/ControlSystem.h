#pragma once

#include <unordered_map>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"

/**
 * System that handles movement controls
 */
class ControlSystem : public Ecs::System
{
public:
	ControlSystem(WorldHolder& worldHolder, const InputData& inputData) noexcept;
	~ControlSystem() override = default;

	void update() override;
	std::string getName() override { return "ControlSystem"; }

private:
	void processPlayerInput();

private:
	WorldHolder& mWorldHolder;
	const InputData& mInputData;
};
