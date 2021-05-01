#include "Base/precomp.h"

#include "ECS/SystemsManager.h"

namespace Ecs
{
	void SystemsManager::update()
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

	void SystemsManager::initResources()
	{
		for (std::unique_ptr<System>& system : mSystems)
		{
			system->initResources();
		}
	}

	void SystemsManager::shutdown()
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

	std::vector<std::string> SystemsManager::getSystemNames()
	{
		std::vector<std::string> result;
		result.reserve(mSystems.size());
		for (std::unique_ptr<System>& system : mSystems)
		{
			result.push_back(system->getName());
		}
		return result;
	}

} // namespace Ecs
