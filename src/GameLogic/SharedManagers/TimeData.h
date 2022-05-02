#pragma once

#include "GameData/Time/GameplayTimestamp.h"

class TimeData
{
public:
	void update(float dt);

public:
	float dt = 0.0f;
	int fixedTimeUpdatesThisFrame = 1;
	GameplayTimestamp currentTimestamp{0};
	u32 frameNumber = 0;
};
