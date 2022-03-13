#pragma once

#include <vector>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Time/GameplayTimestamp.h"

class TravelPath
{
public:
	std::vector<Vector2D> smoothPath;
	Vector2D targetPos{ ZERO_VECTOR };
	GameplayTimestamp updateTimestamp{0};
};
