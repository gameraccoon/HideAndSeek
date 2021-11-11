#include "Base/precomp.h"

#include "Utils/Profiling/SystemFrameRecords.h"

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

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

void SystemFrameRecords::printToFile(const std::vector<std::string>& systemNames, const std::string& fileName, const std::vector<NonFrameTasks>& nonFrameTasks) const
{
	std::ofstream outStream(fileName);
	print(systemNames, outStream, std::move(nonFrameTasks));
}

void SystemFrameRecords::print(const std::vector<std::string>& systemNames, std::ostream& outStream, const std::vector<NonFrameTasks>& nonFrameTasks) const
{
	using RaccoonEcs::AsyncSystemsFrameTime;

	nlohmann::json result;

	result["taskNames"] = systemNames;

	nlohmann::json& frames = result["frames"];

	for (const AsyncSystemsFrameTime& frameRecord : mSystemFrameRecords)
	{
		nlohmann::json frame;
		nlohmann::json& tasks = frame["tasks"];
		for (const AsyncSystemsFrameTime::OneSystemTime& systemTime : frameRecord.systemsTime)
		{
			nlohmann::json task;
			task["threadId"] = systemTime.workerThreadId;
			task["timeStart"] = systemTime.start.time_since_epoch().count();
			task["timeFinish"] = systemTime.end.time_since_epoch().count();
			task["taskNameIdx"] = systemTime.systemIdx;
			tasks.push_back(task);
		}
		frames.push_back(frame);
	}

	nlohmann::json& nonFrameTasksJson = result["nonFrameTasks"];
	for (const NonFrameTasks& taskList : nonFrameTasks)
	{
		result["taskNames"].push_back(taskList.name);
		const size_t taskNameId = result["taskNames"].size() - 1;
		for (const auto& task : taskList.tasks)
		{
			nlohmann::json taskJson;
			taskJson["threadId"] = taskList.threadId;
			taskJson["timeStart"] = task.first.time_since_epoch().count();
			taskJson["timeFinish"] = task.second.time_since_epoch().count();
			taskJson["taskNameIdx"] = taskNameId;
			nonFrameTasksJson.push_back(taskJson);
		}
	}

	outStream << std::setw(4) <<  result;
}
