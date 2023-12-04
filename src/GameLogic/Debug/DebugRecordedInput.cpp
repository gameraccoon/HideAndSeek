#include "Base/precomp.h"

#include "GameLogic/Debug/DebugRecordedInput.h"

#include <format>

#include "Base/Types/Serialization.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/InputControllersData.h"

#include "GameLogic/Game/Game.h"

DebugRecordedInput::DebugRecordedInput(int instanceIndex)
	: mInstanceIndex(instanceIndex)
{
}

DebugRecordedInput::~DebugRecordedInput()
{
	if (mRecordedInputDataFile)
	{
		fclose(mRecordedInputDataFile);
	}
}

bool DebugRecordedInput::processFrameInput(HAL::InputControllersData& gameInputControllersData)
{
	switch (mInputMode)
	{
	case InputMode::Normal:
		break;
	case InputMode::Disable:
		gameInputControllersData = HAL::InputControllersData();
		break;
	case InputMode::Record:
		appendInputDataToFile(gameInputControllersData);
		break;
	case InputMode::Replay:
		if (mRecordedInputDataIdx >= mRecordedInputData.size())
		{
			return mShouldQuitOnEndOfInputData;
		}

		gameInputControllersData = mRecordedInputData[mRecordedInputDataIdx];
		++mRecordedInputDataIdx;
		break;
	default:
		ReportError("Unknown debug input mode");
		break;
	}

	return false;
}

void DebugRecordedInput::processArguments(const ArgumentsParser& arguments)
{
	if (arguments.hasArgument("disable-input"))
	{
		mInputMode = InputMode::Disable;
	}
	else if (arguments.hasArgument("record-input"))
	{
		mInputMode = InputMode::Record;
		const auto argumentOption = arguments.getArgumentValue("record-input");
		if (!argumentOption.has_value())
		{
			ReportError("No file name specified for recording input data");
			return;
		}

		mRecordedInputDataFile = std::fopen((*argumentOption + std::format("_{}.rawinput", mInstanceIndex)).c_str(), "wb");
	}
	else if (arguments.hasArgument("replay-input"))
	{
		mInputMode = InputMode::Replay;
		const auto argumentOption = arguments.getArgumentValue("replay-input");
		if (!argumentOption.has_value())
		{
			ReportError("No file name specified for replaying input data");
			return;
		}
		mRecordedInputData = loadInputDataFromFile(*argumentOption + std::format("_{}.rawinput", mInstanceIndex));
	}

	if (arguments.hasArgument("continue-after-input-end"))
	{
		mShouldQuitOnEndOfInputData = false;
	}
}

void DebugRecordedInput::appendInputDataToFile(const HAL::InputControllersData& inputData)
{
	if (!mRecordedInputDataFile)
	{
		ReportError("No file to record input data");
		return;
	}

	std::vector<std::byte> inputDataBytes;

	// serialize keyboard keys
	const auto& keayboardButtonsBitset = inputData.controllerStates.keyboardState.getPressedButtons();
	const std::byte* keyboardButtonsRawData = keayboardButtonsBitset.getRawData();
	for (size_t i = 0; i < keayboardButtonsBitset.getByteCount(); ++i)
	{
		Serialization::AppendNumber<u8>(inputDataBytes, static_cast<u8>(keyboardButtonsRawData[i]));
	}

	// serialize mouse buttons
	const auto& mouseButtonsBitset = inputData.controllerStates.mouseState.getPressedButtons();
	const std::byte* mouseButtonsRawData = mouseButtonsBitset.getRawData();
	for (size_t i = 0; i < mouseButtonsBitset.getByteCount(); ++i)
	{
		Serialization::AppendNumber<u8>(inputDataBytes, static_cast<u8>(mouseButtonsRawData[i]));
	}

	// serialize mouse axes
	for (size_t i = 0; i < Input::PlayerControllerStates::MouseAxesCount; ++i)
	{
		Serialization::AppendNumber<float>(inputDataBytes, inputData.controllerStates.mouseState.getAxisValue(i));
	}

	// serialize gamepad buttons
	const auto& gamepadButtonsBitset = inputData.controllerStates.gamepadState.getPressedButtons();
	const std::byte* gamepadButtonsRawData = gamepadButtonsBitset.getRawData();
	for (size_t i = 0; i < gamepadButtonsBitset.getByteCount(); ++i)
	{
		Serialization::AppendNumber<u8>(inputDataBytes, static_cast<u8>(gamepadButtonsRawData[i]));
	}

	// serialize gamepad axes
	for (size_t i = 0; i < Input::PlayerControllerStates::GamepadAxesCount; ++i)
	{
		Serialization::AppendNumber<float>(inputDataBytes, inputData.controllerStates.gamepadState.getAxisValue(i));
	}

	std::fwrite(inputDataBytes.data(), sizeof(std::byte), inputDataBytes.size(), mRecordedInputDataFile);
}

std::vector<HAL::InputControllersData> DebugRecordedInput::loadInputDataFromFile(const std::string& path)
{
	std::vector<HAL::InputControllersData> result;
	std::FILE* file = std::fopen(path.c_str(), "rb");
	if (!file)
	{
		ReportError("Failed to open input data file: %s", path.c_str());
		return result;
	}

	std::vector<std::byte> inputDataBytes;
	// read all bytes from file into inputDataBytes
	std::fseek(file, 0, SEEK_END);
	const size_t fileSize = std::ftell(file);
	inputDataBytes.resize(fileSize);
	std::rewind(file);
	const size_t readSize = std::fread(inputDataBytes.data(), sizeof(std::byte), inputDataBytes.size(), file);
	std::fclose(file);

	if (readSize != fileSize)
	{
		ReportError("Incorrect input data file size, expected %d, got %d", fileSize, readSize);
		return result;
	}

	// we reuse it to track just pressed and just released buttons
	HAL::InputControllersData inputData;

	size_t cursorPos = 0;
	while (cursorPos < inputDataBytes.size())
	{
		// deserialize keyboard keys
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::KeyboardButtonCount>::ByteCount> keyboardButtons;
		for (size_t i = 0; i < keyboardButtons.size(); ++i)
		{
			keyboardButtons[i] = static_cast<std::byte>(UnsafeSerialization::ReadNumber<u8>(inputDataBytes, cursorPos));
		}
		inputData.controllerStates.keyboardState.updatePressedButtonsFromRawData(keyboardButtons);

		// deserialize mouse buttons
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::MouseButtonCount>::ByteCount> mouseButtons;
		for (size_t i = 0; i < mouseButtons.size(); ++i)
		{
			mouseButtons[i] = static_cast<std::byte>(UnsafeSerialization::ReadNumber<u8>(inputDataBytes, cursorPos));
		}
		inputData.controllerStates.mouseState.updatePressedButtonsFromRawData(mouseButtons);

		// deserialize mouse axes
		for (size_t i = 0; i < Input::PlayerControllerStates::MouseAxesCount; ++i)
		{
			inputData.controllerStates.mouseState.updateAxis(i, UnsafeSerialization::ReadNumber<float>(inputDataBytes, cursorPos));
		}

		// deserialize gamepad buttons
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::GamepadButtonCount>::ByteCount> gamepadButtons;
		for (size_t i = 0; i < gamepadButtons.size(); ++i)
		{
			gamepadButtons[i] = static_cast<std::byte>(UnsafeSerialization::ReadNumber<u8>(inputDataBytes, cursorPos));
		}
		inputData.controllerStates.gamepadState.updatePressedButtonsFromRawData(gamepadButtons);

		// deserialize gamepad axes
		for (size_t i = 0; i < Input::PlayerControllerStates::GamepadAxesCount; ++i)
		{
			inputData.controllerStates.gamepadState.updateAxis(i, UnsafeSerialization::ReadNumber<float>(inputDataBytes, cursorPos));
		}

		result.push_back(inputData);
	}

	return result;
}
