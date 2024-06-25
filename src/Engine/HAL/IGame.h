#pragma once

#include <chrono>

namespace HAL
{
	class IGame
	{
	public:
		virtual ~IGame() = default;
		// called once before a render frame even if the simulation is paused
		virtual void notPausablePreFrameUpdate(float dt) = 0;
		// called once before batch of updates that needs to be executed in this render frame
		virtual void dynamicTimePreFrameUpdate(float dt, int plannedFixedTimeUpdates) = 0;
		// called per each fixed-time update (can be multiple times per frame)
		virtual void fixedTimeUpdate(float dt) = 0;
		// called once after batch of updates that needs to be executed in this render frame
		virtual void dynamicTimePostFrameUpdate(float dt, int processedFixedTimeUpdates) = 0;
		// called once at the end of a render frame even if the simulation is paused
		virtual void notPausablePostFrameUpdate(float dt) = 0;
		virtual void initResources() = 0;

		virtual bool shouldPauseGame() const = 0;
		virtual bool shouldQuitGame() const = 0;
		virtual void quitGame() = 0;
		virtual std::chrono::duration<s64, std::micro> getFrameLengthCorrection() const = 0;
	};
} // namespace HAL
