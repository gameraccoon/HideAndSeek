#pragma once

#include <unordered_map>

#include "ECS/System.h"
#include "HAL/Base/Engine.h"

#include "HAL/KeyStatesMap.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that handles movement controls
 */
class ControlSystem : public System
{
public:
	ControlSystem(WorldHolder& worldHolder, HAL::Engine* engine, HAL::KeyStatesMap* keyStates);
	~ControlSystem() override = default;

	void update() override;

private:
	WorldHolder& mWorldHolder;
	HAL::Engine* mEngine;
	HAL::KeyStatesMap* mKeyStates;
};