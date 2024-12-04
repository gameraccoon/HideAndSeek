#pragma once

class GameplayTimestamp;

class GameplayTimeDuration
{
public:
	using TimeValueType = s32;

public:
	explicit constexpr GameplayTimeDuration(const TimeValueType time) noexcept
		: mDuration(time) {}

	TimeValueType getFixedFramesCount() const { return mDuration; }

private:
	TimeValueType mDuration = 0;
};
