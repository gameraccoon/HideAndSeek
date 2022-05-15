#pragma once

#include <vector>

#include "GameData/EcsDefinitions.h"

class WorldHolder;
class TimeData;

struct ImguiDebugData
{
	WorldHolder& worldHolder;
	const TimeData& time;
	ComponentFactory& componentFactory;
	std::vector<std::string> systemNames;
};
