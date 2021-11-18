#pragma once

#include <string>
#include <vector>
#include <chrono>

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

	struct ProfileData
	{
		std::vector<std::string> threadNames;
		ScopedProfilerDatas scopedProfilerDatas;
	};

	static void PrintToFile(const std::string& fileName, const ProfileData& profileData);
	static void Print(std::ostream& stream, const ProfileData& profilerData);
};
