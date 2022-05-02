#include "Base/precomp.h"

#include "GameLogic/SharedManagers/TimeData.h"

#include <cmath>

void TimeData::fixedUpdate(float deltaTime, u32 updatesCount)
{
	lastFixedUpdateDt = deltaTime;
	lastUpdateDt = deltaTime;
	lastFixedUpdateTimestamp.increaseByFloatTime(deltaTime);
	countFixedTimeUpdatesThisFrame += updatesCount;
}
