#pragma once

#include <vector>

#include "GameData/EcsDefinitions.h"

class WorldHolder;
class SystemFrameRecords;
class TimeData;

struct ImguiDebugData
{
	WorldHolder& worldHolder;
	const TimeData& time;
	SystemFrameRecords& systemRecords;
	ComponentFactory& componentFactory;
	std::vector<std::string> systemNames;
};
