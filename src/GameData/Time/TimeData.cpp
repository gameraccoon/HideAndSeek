#include "Base/precomp.h"

#include "GameData/Time/TimeData.h"

void TimeData::fixedUpdate(float deltaTime, u32 updatesCount)
{
	lastFixedUpdateDt = deltaTime;
	lastUpdateDt = deltaTime;
	lastFixedUpdateTimestamp.increaseByFloatTime(deltaTime);
	countFixedTimeUpdatesThisFrame += updatesCount;
	++lastFixedUpdateIndex;
}
