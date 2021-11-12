#pragma once

#include <string>

#include "Base/Profile/ScopedProfiler.h"

#include <raccoon-ecs/async_systems_manager.h>

class SystemFrameRecords
{
public:
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	struct NonFrameTask
	{
		size_t threadId;
		std::string taskName;
		std::vector<std::pair<TimePoint, TimePoint>> taskInstances;
	};
	using NonFrameTasks = std::vector<NonFrameTask>;

	struct ScopedProfilerData
	{
		size_t threadId;
		ScopedProfilerThreadData::Records records;
	};
	using ScopedProfilerDatas = std::vector<ScopedProfilerData>;

	void setRecordsLimit(unsigned int newLimit);

	void addFrame(RaccoonEcs::AsyncSystemsFrameTime&& frameTime);
	std::vector<RaccoonEcs::AsyncSystemsFrameTime>& getFramesRef();

	void pauseRecording();
	void resumeRecording();
	[[nodiscard]] bool isRecordingActive() const;

	void printToFile(const std::vector<std::string>& systemNames, const std::string& fileName, const NonFrameTasks& nonFrameTasks, const ScopedProfilerDatas& scopedProfilerDatas) const;
	void print(const std::vector<std::string>& systemNames, std::ostream& stream, const NonFrameTasks& nonFrameTasks, const ScopedProfilerDatas& scopedProfilerDatas) const;

private:
	std::vector<RaccoonEcs::AsyncSystemsFrameTime> mSystemFrameRecords;
	unsigned int mRecordsLimit = 0;
	bool mIsRecordingActive = true;
};
