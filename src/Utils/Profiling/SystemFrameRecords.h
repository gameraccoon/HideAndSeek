#pragma once

#include <string>

#include <raccoon-ecs/async_systems_manager.h>

class SystemFrameRecords
{
public:
	SystemFrameRecords();

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

	struct ProfileData
	{
		std::vector<std::string> systemNames;
		std::vector<std::string> threadNames;
		NonFrameTasks nonFrameTasks;
		ScopedProfilerDatas scopedProfilerDatas;
	};

	void setRecordsLimit(unsigned int newLimit);

	void addFrame(RaccoonEcs::AsyncSystemsFrameTime&& frameTime);
	std::vector<RaccoonEcs::AsyncSystemsFrameTime>& getFramesRef();

	void pauseRecording();
	void resumeRecording();
	[[nodiscard]] bool isRecordingActive() const;

	void printToFile(const std::string& fileName, const ProfileData& profileData) const;
	void print(std::ostream& stream, const ProfileData& profilerData) const;

private:
	double getTimeMicrosecondsFromPoint(const std::chrono::time_point<std::chrono::system_clock>& timePoint) const;

private:
	std::vector<RaccoonEcs::AsyncSystemsFrameTime> mSystemFrameRecords;
	unsigned int mRecordsLimit = 0;
	bool mIsRecordingActive = true;
	const long mCreationTimeNs;
};
