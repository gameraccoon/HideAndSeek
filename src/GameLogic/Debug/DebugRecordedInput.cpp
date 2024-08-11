#include "EngineCommon/precomp.h"

#include "GameLogic/Debug/DebugRecordedInput.h"

#include <format>

#include "EngineCommon/Types/Serialization.h"

#include "HAL/InputControllersData.h"

#include "GameUtils/Application/ArgumentsParser.h"

DebugRecordedInput::DebugRecordedInput(const int instanceIndex)
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

		const std::string& fileName = *argumentOption + std::format("_{}.rawinput", mInstanceIndex);

		mRecordedInputDataFile = std::fopen(fileName.c_str(), "wb");
		AssertFatal(mRecordedInputDataFile, "Failed to open file for recording input data %s", fileName.c_str());
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

void DebugRecordedInput::appendInputDataToFile(const HAL::InputControllersData& inputData) const
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
	constexpr size_t oneInputFrameSize =
		BitsetTraits<Input::PlayerControllerStates::KeyboardButtonCount>::ByteCount
		+ BitsetTraits<Input::PlayerControllerStates::MouseButtonCount>::ByteCount
		+ Input::PlayerControllerStates::MouseAxesCount * sizeof(float)
		+ BitsetTraits<Input::PlayerControllerStates::GamepadButtonCount>::ByteCount
		+ Input::PlayerControllerStates::GamepadAxesCount * sizeof(float);

	while (cursorPos + oneInputFrameSize - 1 < inputDataBytes.size())
	{
		// deserialize keyboard keys
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::KeyboardButtonCount>::ByteCount> keyboardButtons;
		for (size_t i = 0; i < keyboardButtons.size(); ++i)
		{
			const auto valueOption = Serialization::ReadNumber<u8>(inputDataBytes, cursorPos);
			AssertFatal(valueOption.has_value(), "Failed to read keyboard button value");
			keyboardButtons[i] = static_cast<std::byte>(*valueOption);
		}
		inputData.controllerStates.keyboardState.updatePressedButtonsFromRawData(keyboardButtons);

		// deserialize mouse buttons
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::MouseButtonCount>::ByteCount> mouseButtons;
		for (size_t i = 0; i < mouseButtons.size(); ++i)
		{
			const auto valueOption = Serialization::ReadNumber<u8>(inputDataBytes, cursorPos);
			AssertFatal(valueOption.has_value(), "Failed to read mouse button value");
			mouseButtons[i] = static_cast<std::byte>(*valueOption);
		}
		inputData.controllerStates.mouseState.updatePressedButtonsFromRawData(mouseButtons);

		// deserialize mouse axes
		for (size_t i = 0; i < Input::PlayerControllerStates::MouseAxesCount; ++i)
		{
			const auto valueOption = Serialization::ReadNumber<float>(inputDataBytes, cursorPos);
			AssertFatal(valueOption.has_value(), "Failed to read mouse axis value");
			inputData.controllerStates.mouseState.updateAxis(i, *valueOption);
		}

		// deserialize gamepad buttons
		std::array<std::byte, BitsetTraits<Input::PlayerControllerStates::GamepadButtonCount>::ByteCount> gamepadButtons;
		for (size_t i = 0; i < gamepadButtons.size(); ++i)
		{
			const auto valueOption = Serialization::ReadNumber<u8>(inputDataBytes, cursorPos);
			AssertFatal(valueOption.has_value(), "Failed to read gamepad button value");
			gamepadButtons[i] = static_cast<std::byte>(*valueOption);
		}
		inputData.controllerStates.gamepadState.updatePressedButtonsFromRawData(gamepadButtons);

		// deserialize gamepad axes
		for (size_t i = 0; i < Input::PlayerControllerStates::GamepadAxesCount; ++i)
		{
			const auto valueOption = Serialization::ReadNumber<float>(inputDataBytes, cursorPos);
			AssertFatal(valueOption.has_value(), "Failed to read gamepad axis value");
			inputData.controllerStates.gamepadState.updateAxis(i, *valueOption);
		}

		result.push_back(inputData);
	}

	return result;
}
