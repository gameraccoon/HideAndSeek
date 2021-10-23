#pragma once

#include <string>

#include <raccoon-ecs/async_systems_manager.h>

class SystemFrameRecords
{
public:
	void setRecordsLimit(unsigned int newLimit);

	void addFrame(RaccoonEcs::AsyncSystemsFrameTime&& frameTime);
	std::vector<RaccoonEcs::AsyncSystemsFrameTime>& getFramesRef();

	void pauseRecording();
	void resumeRecording();
	[[nodiscard]] bool isRecordingActive() const;

	void printToFile(const std::vector<std::string>& systemNames, const std::string& fileName) const;
	void print(const std::vector<std::string>& systemNames, std::ostream& stream) const;

private:
	std::vector<RaccoonEcs::AsyncSystemsFrameTime> mSystemFrameRecords;
	unsigned int mRecordsLimit = 0;
	bool mIsRecordingActive = true;
};
