#pragma once

#ifdef ENABLE_SCOPED_PROFILER

#include <chrono>
#include <string>
#include <vector>

#include "EngineCommon/Profile/ScopedProfiler.h"

class ProfileDataWriter
{
public:
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	struct ScopedProfilerData
	{
		size_t threadId;
		ScopedProfilerThreadData::Records records;
	};
	using ScopedProfilerDatas = std::vector<ScopedProfilerData>;
	using FrameDurations = std::vector<double>;

	struct ProfileData
	{
		std::vector<std::string> threadNames;
		ScopedProfilerDatas scopedProfilerDatas;
	};

	static void PrintScopedProfileToFile(const std::string& fileName, const ProfileData& profileData);
	static void PrintScopedProfile(std::ostream& outStream, const ProfileData& profilerData);

	static void PrintFrameDurationStatsToFile(const std::string& fileName, const FrameDurations& frameTimes);
	static void PrintFrameDurationStats(std::ostream& outStream, const FrameDurations& frameTimes);
};

#endif // ENABLE_SCOPED_PROFILER
