#pragma once

#include "AutoTests/TestCheckList.h"

// The code of the test can set this type of check as failed or succeeded
class SimpleTestCheck final : public TestCheck
{
public:
	SimpleTestCheck(const SimpleTestCheck&) = delete;
	SimpleTestCheck(SimpleTestCheck&&) = delete;

	template<typename Str, typename = std::enable_if_t<std::is_convertible_v<Str, std::string>>>
	explicit SimpleTestCheck(Str&& errorMessage)
		: mErrorMessage(std::forward<Str>(errorMessage))
	{}

	[[nodiscard]] bool hasBeenValidated() const override { return mHasBeenValidated; }
	[[nodiscard]] bool hasPassed() const final { return mHasPassed; }
	[[nodiscard]] std::string getErrorMessage() const final { return mErrorMessage; }

	void checkAsPassed()
	{
		mHasPassed = true;
		mHasBeenValidated = true;
	}

	void checkAsFailed()
	{
		mHasPassed = false;
		mHasBeenValidated = true;
	}

private:
	bool mHasPassed = false;
	bool mHasBeenValidated = false;
	std::string mErrorMessage;
};

// Call update() in the game loop, and it will fail after the specified number of frames
class TimeoutCheck final : public TestCheck
{
public:
	explicit TimeoutCheck(size_t timeoutFrames)
		: mTimeoutFrames(timeoutFrames)
	{}

	void update()
	{
		++mFramesCount;
	}

	[[nodiscard]] bool hasPassed() const final
	{
		return mFramesCount < mTimeoutFrames;
	}

	[[nodiscard]] bool hasBeenValidated() const final
	{
		return true;
	}

	[[nodiscard]] std::string getErrorMessage() const final
	{
		return "The test hasn't been completed in " + std::to_string(mTimeoutFrames) + " frames";
	}

private:
	const size_t mTimeoutFrames = 0;
	size_t mFramesCount = 0;
};
