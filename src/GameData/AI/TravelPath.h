#pragma once

#include <vector>

#include "EngineData/Geometry/Vector2D.h"

#include "EngineData/Time/GameplayTimestamp.h"

class TravelPath
{
public:
	std::vector<Vector2D> smoothPath;
	Vector2D targetPos{ ZERO_VECTOR };
	GameplayTimestamp updateTimestamp{ 0 };
};
