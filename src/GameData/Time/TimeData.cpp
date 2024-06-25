#include "EngineCommon/precomp.h"

#include "GameData/Time/TimeData.h"

void TimeData::fixedUpdate(const float deltaTime, const u32 updatesCount)
{
	lastFixedUpdateDt = deltaTime;
	lastUpdateDt = deltaTime;
	lastFixedUpdateTimestamp.increaseByUpdateCount(updatesCount);
	++lastFixedUpdateIndex;
}
