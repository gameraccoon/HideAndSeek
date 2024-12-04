#pragma once

#include <vector>

#include "EngineData/Time/TimeData.h"

#include "GameData/EcsDefinitions.h"

class WorldHolder;

struct ImguiDebugData
{
	WorldHolder& worldHolder;
	ComponentFactory& componentFactory;
	std::vector<std::string> systemNames;
	TimeData time;
};
