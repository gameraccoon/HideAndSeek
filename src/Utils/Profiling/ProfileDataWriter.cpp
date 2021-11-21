#include "Base/precomp.h"

#ifdef ENABLE_SCOPED_PROFILER
#include "Utils/Profiling/ProfileDataWriter.h"

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

void ProfileDataWriter::PrintToFile(const std::string& fileName, const ProfileData& profileData)
{
	std::ofstream outStream(fileName);
	Print(outStream, profileData);
}

static long getTimeNsFromPoint(const std::chrono::time_point<std::chrono::system_clock>& timePoint) {
	// try to reduce floating point error by making the time relative to the app start befor converting to fp value
	return std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint.time_since_epoch()).count();
}

void ProfileDataWriter::Print(std::ostream& outStream, const ProfileData& profileData)
{
	// time of latest first record, we cut any earlier events to keep the records consistent
	long startTimeNs = 0;

	for (const ScopedProfilerData& scopeData : profileData.scopedProfilerDatas)
	{
		long threadFirstTime = std::numeric_limits<long>::max();
		for (const ScopedProfilerThreadData::ScopeRecord& record : scopeData.records)
		{
			if (record.stackDepth != ScopedProfilerThreadData::InvalidStackDepth)
			{
				threadFirstTime = std::min(threadFirstTime, getTimeNsFromPoint(record.begin));
			}
		}
		if (threadFirstTime != std::numeric_limits<long>::max())
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
			if (record.stackDepth != ScopedProfilerThreadData::InvalidStackDepth)
			{
				long recordTimeBeginNs = getTimeNsFromPoint(record.begin);
				const long recordTimeEndNs = getTimeNsFromPoint(record.end);
				if (recordTimeEndNs > startTimeNs)
				{
					nlohmann::json taskBegin;
					taskBegin["name"] = record.scopeName;
					taskBegin["ph"] = "B";
					taskBegin["pid"] = 1;
					taskBegin["tid"] = scopeData.threadId;
					taskBegin["ts"] = (recordTimeBeginNs  - startTimeNs) * 0.001;
					taskBegin["sf"] = std::to_string(scopeData.threadId) + "#" + std::to_string(record.stackDepth) + "#" + record.scopeName;
					events.push_back(taskBegin);

					nlohmann::json taskEnd;
					taskEnd["name"] = record.scopeName;
					taskEnd["ph"] = "E";
					taskEnd["pid"] = 1;
					taskEnd["tid"] = scopeData.threadId;
					taskEnd["ts"] = (recordTimeEndNs  - startTimeNs) * 0.001;
					events.push_back(taskEnd);
				}
			}
		}
	}

	std::sort(
		events.begin(),
		events.end(),
		[](const nlohmann::json& eventJsonL, const nlohmann::json& eventJsonR)
		{
			return eventJsonL.at("ts").get<double>() < eventJsonR.at("ts").get<double>();
		}
	);

	const double minTime = events.begin()->at("ts").get<double>();

	std::for_each(
		events.begin(),
		events.end(),
		[minTime](nlohmann::json& eventJson){
			eventJson["ts"] = eventJson["ts"].get<double>() - minTime;
		}
	);

	for (size_t i = 0; i < profileData.threadNames.size(); ++i)
	{
		const std::string& threadName = profileData.threadNames[i];
		events.insert(events.begin(), nlohmann::json{
			{ "name", "thread_name" },
			{ "ph", "M" },
			{ "pid", 1 },
			{ "tid", i },
			{ "args", nlohmann::json{ {"name", threadName} }}
		});
	}

	outStream << std::setw(4) <<  result;
}
#endif // ENABLE_SCOPED_PROFILER
