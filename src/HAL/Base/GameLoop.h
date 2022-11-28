#pragma once

#include <thread>

#include "HAL/Base/Constants.h"
#include "HAL/IGame.h"

namespace HAL
{
	template<typename ShouldStopFnT = nullptr_t, typename OnIterationFnT = nullptr_t>
	void RunGameLoop(IGame& game, ShouldStopFnT&& shouldStopFn = nullptr, OnIterationFnT&& onIterationFn = nullptr)
	{
		constexpr auto oneFrameDuration = Constants::ONE_FIXED_UPDATE_DURATION;

		auto lastFrameTime = std::chrono::steady_clock::now() - oneFrameDuration;

		while (!game.shouldQuitGame())
		{
			if constexpr (!std::is_same_v<ShouldStopFnT, nullptr_t>)
			{
				 if (shouldStopFn())
				 {
					 break;
				 }
			}

			if constexpr (!std::is_same_v<OnIterationFnT, nullptr_t>)
			{
				onIterationFn();
			}

			auto timeNow = std::chrono::steady_clock::now();

			int iterationsThisFrame = 0;
			auto passedTime = timeNow - lastFrameTime;
			if (passedTime >= oneFrameDuration)
			{
				// if we exceeded max frame ticks last frame, that likely mean we were staying on a breakpoint
				// so, let's readjust to normal ticking speed
				if (passedTime > Constants::MAX_FRAME_DURATION)
				{
					passedTime = Constants::ONE_FIXED_UPDATE_DURATION;
				}

				const float lastFrameDurationSec = std::chrono::duration<float>(passedTime).count();

				while (passedTime >= oneFrameDuration)
				{
					passedTime -= oneFrameDuration;
					++iterationsThisFrame;
				}

				game.dynamicTimePreFrameUpdate(lastFrameDurationSec, iterationsThisFrame);
				for (int i = 0; i < iterationsThisFrame; ++i)
				{
					game.fixedTimeUpdate(Constants::ONE_FIXED_UPDATE_SEC);
				}
				game.dynamicTimePostFrameUpdate(lastFrameDurationSec, iterationsThisFrame);

				lastFrameTime = timeNow - passedTime;
			}

			if (iterationsThisFrame <= 1)
			{
				std::this_thread::yield();
			}
		}
	}
}
