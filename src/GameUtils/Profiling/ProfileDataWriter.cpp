#include "EngineCommon/precomp.h"

#ifdef ENABLE_SCOPED_PROFILER

#include <fstream>
#include <iomanip>

#include <nlohmann/json.hpp>

#include "GameUtils/Profiling/ProfileDataWriter.h"

void ProfileDataWriter::PrintScopedProfileToFile(const std::string& fileName, const ProfileData& profileData)
{
	std::ofstream outStream(fileName);
	PrintScopedProfile(outStream, profileData);
}

void ProfileDataWriter::PrintScopedProfile(std::ostream& outStream, const ProfileData& profileData)
{
	// time of latest first record, we cut any earlier events to keep the records consistent
	auto startTimeNs = std::chrono::time_point<std::chrono::system_clock>::min();

	for (const ScopedProfilerData& scopeData : profileData.scopedProfilerDatas)
	{
		auto threadFirstTime = std::chrono::time_point<std::chrono::system_clock>::max();
		for (const ScopedProfilerThreadData::ScopeRecord& record : scopeData.records)
		{
			if (record.scopeName != nullptr)
			{
				threadFirstTime = std::min(threadFirstTime, record.begin);
			}
		}

		if (threadFirstTime != std::chrono::time_point<std::chrono::system_clock>::max())
		{
			startTimeNs = std::max(startTimeNs, threadFirstTime);
		}
	}

	nlohmann::json result;
	nlohmann::json& events = result["traceEvents"];

	for (const ScopedProfilerData& scopeData : profileData.scopedProfilerDatas)
	{
		for (const ScopedProfilerThreadData::ScopeRecord& record : scopeData.records)
		{
			if (record.scopeName != nullptr)
			{
				if (record.end > startTimeNs)
				{
					nlohmann::json taskBegin;
					taskBegin["name"] = record.scopeName;
					taskBegin["ph"] = "B";
					taskBegin["pid"] = 1;
					taskBegin["tid"] = scopeData.threadId;
					taskBegin["ts"] = std::chrono::duration<double, std::micro>(record.begin - startTimeNs).count();
					taskBegin["sf"] = std::to_string(scopeData.threadId) + "#" + record.scopeName;
					events.push_back(taskBegin);

					nlohmann::json taskEnd;
					taskEnd["name"] = record.scopeName;
					taskEnd["ph"] = "E";
					taskEnd["pid"] = 1;
					taskEnd["tid"] = scopeData.threadId;
					taskEnd["ts"] = std::chrono::duration<double, std::micro>(record.end - startTimeNs).count();
					events.push_back(taskEnd);
				}
			}
		}
	}

	std::sort(
		events.begin(),
		events.end(),
		[](const nlohmann::json& eventJsonL, const nlohmann::json& eventJsonR) {
			return eventJsonL.at("ts").get<double>() < eventJsonR.at("ts").get<double>();
		}
	);

	const double minTime = events.begin()->at("ts").get<double>();

	std::for_each(
		events.begin(),
		events.end(),
		[minTime](nlohmann::json& eventJson) {
			eventJson["ts"] = eventJson["ts"].get<double>() - minTime;
		}
	);

	for (size_t i = 0; i < profileData.threadNames.size(); ++i)
	{
		const std::string& threadName = profileData.threadNames[i];
		events.insert(events.begin(), nlohmann::json{ { "name", "thread_name" }, { "ph", "M" }, { "pid", 1 }, { "tid", i }, { "args", nlohmann::json{ { "name", threadName } } } });
	}

	outStream << std::setw(4) << result;
}

void ProfileDataWriter::PrintFrameDurationStatsToFile(const std::string& fileName, const FrameDurations& frameTimes)
{
	std::ofstream outStream(fileName);
	PrintFrameDurationStats(outStream, frameTimes);
}

void ProfileDataWriter::PrintFrameDurationStats(std::ostream& outStream, const FrameDurations& frameTimes)
{
	bool isFirst = true;
	for (const double frameTime : frameTimes)
	{
		if (!isFirst)
		{
			outStream << '\n';
		}
		else
		{
			isFirst = false;
		}
		outStream << frameTime;
	}
	outStream << '\n';
}

#endif // ENABLE_SCOPED_PROFILER
