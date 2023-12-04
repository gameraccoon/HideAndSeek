#pragma once

#include <cstdio>

#include "GameData/Input/GameplayInputConstants.h"

class Game;
class ArgumentsParser;

namespace HAL
{
	class InputControllersData;
}

class DebugRecordedInput
{
public:
	DebugRecordedInput(int instanceIndex);

	~DebugRecordedInput();

	// returns true if game should quit
	bool processFrameInput(HAL::InputControllersData& gameInputControllersData);
	void processArguments(const ArgumentsParser& arguments);

private:
	enum class InputMode
	{
		Normal,
		Disable,
		Record,
		Replay
	};

private:
	void appendInputDataToFile(const HAL::InputControllersData& inputData);
	static std::vector<HAL::InputControllersData> loadInputDataFromFile(const std::string& path);

private:
	InputMode mInputMode = InputMode::Normal;
	std::vector<HAL::InputControllersData> mRecordedInputData;
	size_t mRecordedInputDataIdx = 0;

	// right now STL doesn't support reading std::byte from file, so we resort to C-style file IO
	std::FILE* mRecordedInputDataFile = nullptr;

	int mInstanceIndex = 0;
	bool mShouldQuitOnEndOfInputData = true;
};
