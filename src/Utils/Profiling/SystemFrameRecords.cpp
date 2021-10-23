#include "Base/precomp.h"

#include "Utils/Profiling/SystemFrameRecords.h"

#include <fstream>

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

void SystemFrameRecords::printToFile(const std::vector<std::string>& systemNames, const std::string& fileName) const
{
	std::ofstream outStream(fileName);
	print(systemNames, outStream);
}

void SystemFrameRecords::print(const std::vector<std::string>& systemNames, std::ostream& outStream) const
{
	// printing in CSV format
	outStream << "\"Total\"";
	for (const auto& systemName : systemNames)
	{
		outStream << ",\"" << systemName << "\"";
	}
	outStream << "\n";

	for (const auto& frameRecord : mSystemFrameRecords)
	{
		outStream << frameRecord.frameTime.count();
		for (const auto& systemTime : frameRecord.systemsTime)
		{
			outStream << "," << (systemTime.end - systemTime.start).count();
		}
		outStream << "\n";
	}
}
