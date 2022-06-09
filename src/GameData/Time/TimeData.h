#pragma once

#include "GameData/Time/GameplayTimestamp.h"

class TimeData
{
public:
	void fixedUpdate(float dt, u32 updatesCount = 1);

public:
	float lastUpdateDt = 0.0f;
	float lastFixedUpdateDt = 0.0f;
	GameplayTimestamp lastFixedUpdateTimestamp{0};
	u32 lastFixedUpdateIndex = 0;
	int countFixedTimeUpdatesThisFrame = 1;
};
