#pragma once

#include <vector>

#include "GameData/EcsDefinitions.h"
#include "GameData/Time/TimeData.h"

class WorldHolder;

struct ImguiDebugData
{
	WorldHolder& worldHolder;
	ComponentFactory& componentFactory;
	std::vector<std::string> systemNames;
	TimeData time;
};
