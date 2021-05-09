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

		void update()
		{
#ifdef PROFILE_SYSTEMS
			mThisFrameTime.frameTime = std::chrono::microseconds::zero();
			mThisFrameTime.systemsTime.clear();
#endif // PROFILE_SYSTEMS

			for (std::unique_ptr<System>& system : mSystems)
			{
#ifdef PROFILE_SYSTEMS
				std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
#endif // PROFILE_SYSTEMS

				// real work is being done here
				system->update();

#ifdef PROFILE_SYSTEMS
				std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
				auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				mThisFrameTime.frameTime += timeDiff;
				mThisFrameTime.systemsTime.push_back(timeDiff);
#endif // PROFILE_SYSTEMS
			}

#ifdef PROFILE_SYSTEMS
			mPreviousFrameTime = mThisFrameTime;
#endif // PROFILE_SYSTEMS
		}

		void initResources()
		{
			for (std::unique_ptr<System>& system : mSystems)
			{
				system->initResources();
			}
		}

		void shutdown()
		{
			for (std::unique_ptr<System>& system : mSystems)
			{
				system->shutdown();
			}
			mSystems.clear();
		}

#ifdef PROFILE_SYSTEMS
	SystemsFrameTime SystemsManager::getPreviousFrameTimeData()
	{
		return mPreviousFrameTime;
	}
#endif // PROFILE_SYSTEMS

	std::vector<std::string> getSystemNames()
	{
		std::vector<std::string> result;
		result.reserve(mSystems.size());
		for (std::unique_ptr<System>& system : mSystems)
		{
			result.push_back(system->getName());
		}
		return result;
	}

	private:
		std::vector<std::unique_ptr<System>> mSystems;

#ifdef PROFILE_SYSTEMS
		SystemsFrameTime mThisFrameTime;
		SystemsFrameTime mPreviousFrameTime;
#endif // PROFILE_SYSTEMS
	};

} // namespace Ecs
