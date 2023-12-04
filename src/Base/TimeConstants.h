#pragma once

#include <chrono>

#include "Base/Types/BasicTypes.h"

namespace TimeConstants
{
	// note that instead of counting in fractions of seconds we count in integral milliseconds/ticks
	// so the frame rate is not exactly 60 FPS and the frame time is not exactly 1/60 sec
	static constexpr int ONE_SECOND_TICKS = 1000;
	static constexpr float ONE_TICK_SECONDS = 1.0f / ONE_SECOND_TICKS;
	static constexpr u64 ONE_FIXED_UPDATE_TICKS = 16;
	static constexpr float ONE_FIXED_UPDATE_SEC = ONE_FIXED_UPDATE_TICKS * ONE_TICK_SECONDS;

	static constexpr std::chrono::duration ONE_FIXED_UPDATE_DURATION = std::chrono::milliseconds{16};
	static constexpr std::chrono::duration MAX_FRAME_DURATION = std::chrono::milliseconds{1000};
}
