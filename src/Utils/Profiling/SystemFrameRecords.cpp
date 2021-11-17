#include "Base/precomp.h"

#include "Utils/Profiling/SystemFrameRecords.h"

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

SystemFrameRecords::SystemFrameRecords()
	: mCreationTimeNs(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
{
}

void SystemFrameRecords::setRecordsLimit(unsigned int newLimit)
{
	mRecordsLimit = newLimit;
}

void SystemFrameRecords::addFrame(RaccoonEcs::AsyncSystemsFrameTime&& frameTime)
{
	if (!mIsRecordingActive)
	{
		return;
	}

	if (mRecordsLimit != 0 && mSystemFrameRecords.size() + 1 > mRecordsLimit)
	{
		mSystemFrameRecords.erase(mSystemFrameRecords.begin());
	}

	mSystemFrameRecords.emplace_back(std::move(frameTime));
}

std::vector<RaccoonEcs::AsyncSystemsFrameTime>& SystemFrameRecords::getFramesRef()
{
	return mSystemFrameRecords;
}

void SystemFrameRecords::pauseRecording()
{
	mIsRecordingActive = false;
}

void SystemFrameRecords::resumeRecording()
{
	mIsRecordingActive = true;
}

bool SystemFrameRecords::isRecordingActive() const
{
	return mIsRecordingActive;
}

void SystemFrameRecords::printToFile(const std::string& fileName, const ProfileData& profileData) const
{
	std::ofstream outStream(fileName);
	print(outStream, profileData);
}

double SystemFrameRecords::getTimeMicrosecondsFromPoint(const std::chrono::time_point<std::chrono::system_clock>& timePoint) const
{
	// try to reduce floating point error by making the time relative to the app start befor converting to fp value
	return (std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint.time_since_epoch()).count() - mCreationTimeNs) * 0.001;
}

void SystemFrameRecords::print(std::ostream& outStream, const ProfileData& profileData) const
{
	using RaccoonEcs::AsyncSystemsFrameTime;

	nlohmann::json result;

	nlohmann::json& events = result["traceEvents"];

	for (const AsyncSystemsFrameTime& frameRecord : mSystemFrameRecords)
	{
		for (const AsyncSystemsFrameTime::OneSystemTime& systemTime : frameRecord.systemsTime)
		{
			nlohmann::json taskBegin;
			taskBegin["name"] = profileData.systemNames[systemTime.systemIdx];
			taskBegin["ph"] = "B";
			taskBegin["pid"] = 1;
			taskBegin["tid"] = systemTime.workerThreadId;
			taskBegin["ts"] = getTimeMicrosecondsFromPoint(systemTime.begin);
			events.push_back(taskBegin);

			nlohmann::json taskEnd;
			taskEnd["name"] = profileData.systemNames[systemTime.systemIdx];
			taskEnd["ph"] = "E";
			taskEnd["pid"] = "1";
			taskEnd["tid"] = systemTime.workerThreadId;
			taskEnd["ts"] = getTimeMicrosecondsFromPoint(systemTime.end);
			events.push_back(taskEnd);
		}
	}

	for (const NonFrameTask& taskList : profileData.nonFrameTasks)
	{
		for (const auto& task : taskList.taskInstances)
		{
			nlohmann::json taskBegin;
			taskBegin["name"] = taskList.taskName;
			taskBegin["ph"] = "B";
			taskBegin["pid"] = 1;
			taskBegin["tid"] = taskList.threadId;
			taskBegin["ts"] = getTimeMicrosecondsFromPoint(task.first);
			events.push_back(taskBegin);

			nlohmann::json taskEnd;
			taskEnd["name"] = taskList.taskName;
			taskEnd["ph"] = "E";
			taskEnd["pid"] = 1;
			taskEnd["tid"] = taskList.threadId;
			taskEnd["ts"] = getTimeMicrosecondsFromPoint(task.second);
			events.push_back(taskEnd);
		}
	}

	for (const ScopedProfilerData& scopeData : profileData.scopedProfilerDatas)
	{
		for (const ScopedProfilerThreadData::ScopeRecord& record : scopeData.records)
		{
			if (record.stackDepth != ScopedProfilerThreadData::InvalidStackDepth)
			{
				nlohmann::json taskBegin;
				taskBegin["name"] = record.scopeName;
				taskBegin["ph"] = "B";
				taskBegin["pid"] = 1;
				taskBegin["tid"] = scopeData.threadId;
				taskBegin["ts"] = getTimeMicrosecondsFromPoint(record.begin);
				taskBegin["sf"] = std::to_string(scopeData.threadId) + "#" + std::to_string(record.stackDepth) + "#" + record.scopeName;
				events.push_back(taskBegin);

				nlohmann::json taskEnd;
				taskEnd["name"] = record.scopeName;
				taskEnd["ph"] = "E";
				taskEnd["pid"] = 1;
				taskEnd["tid"] = scopeData.threadId;
				taskEnd["ts"] = getTimeMicrosecondsFromPoint(record.end);
				events.push_back(taskEnd);
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
