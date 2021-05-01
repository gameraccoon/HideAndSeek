#pragma once

#include <string>

#include "ECS/SystemsManager.h"

class SystemFrameRecords
{
public:
	void setRecordsLimit(unsigned int newLimit);

	void addFrame(Ecs::SystemsFrameTime&& frameTime);
	std::vector<Ecs::SystemsFrameTime>& getFramesRef();

	void pauseRecording();
	void resumeRecording();
	[[nodiscard]] bool isRecordingActive() const;

	void printToFile(const std::vector<std::string>& systemNames, const std::string& fileName) const;
	void print(const std::vector<std::string>& systemNames, std::ostream& stream) const;

private:
	std::vector<Ecs::SystemsFrameTime> mSystemFrameRecords;
	unsigned int mRecordsLimit = 0;
	bool mIsRecordingActive = true;
};
