#include "Base/precomp.h"

#include "GameLogic/SharedManagers/TimeData.h"

#include <cmath>

void TimeData::update(float deltaTime)
{
	dt = deltaTime;
	currentTimestamp.increaseByFloatTime(dt);
}
