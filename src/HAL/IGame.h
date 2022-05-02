#pragma once

namespace HAL
{
	class IGame
	{
	public:
		virtual ~IGame() = default;
		virtual void dynamicTimePreFrameUpdate(float dt, int plannedFixedTimeUpdates) = 0;
		virtual void fixedTimeUpdate(float dt) = 0;
		virtual void dynamicTimePostFrameUpdate(float dt, int processedFixedTimeUpdates) = 0;
		virtual void initResources() = 0;

		virtual void quitGame() = 0;
		virtual bool shouldQuitGame() const = 0;
	};
}
