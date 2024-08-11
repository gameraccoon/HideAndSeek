#pragma once

#include "GameData/Time/GameplayTimestamp.h"

struct TimeData
{
	float lastUpdateDt = 0.0f;
	float lastFixedUpdateDt = 0.0f;
	GameplayTimestamp lastFixedUpdateTimestamp{ 0 };
	u32 lastFixedUpdateIndex = 0;
	int countFixedTimeUpdatesThisFrame = 1;

	void fixedUpdate(float deltaTime, u32 updatesCount = 1);
};
