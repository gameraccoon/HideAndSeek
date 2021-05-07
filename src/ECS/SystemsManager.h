#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "System.h"

namespace Ecs
{
	struct SystemsFrameTime
	{
		std::chrono::microseconds frameTime;
		std::vector<std::chrono::microseconds> systemsTime;
	};

	/**
	 * Manager for game systems
	 */
	class SystemsManager
	{
	public:
		template <typename T, typename... Args>
		void registerSystem(Args&&... args)
		{
			mSystems.emplace_back(new T(std::forward<Args>(args)...));
		}

		void update();
		void initResources();
		void shutdown();

#ifdef PROFILE_SYSTEMS
		SystemsFrameTime getPreviousFrameTimeData();
#endif // PROFILE_SYSTEMS
		std::vector<std::string> getSystemNames();

	private:
		std::vector<std::unique_ptr<System>> mSystems;
#ifdef PROFILE_SYSTEMS
		SystemsFrameTime mThisFrameTime;
		SystemsFrameTime mPreviousFrameTime;
#endif // PROFILE_SYSTEMS
	};

} // namespace Ecs
