#pragma once

#include <limits>
#include <nlohmann/json_fwd.hpp>

#include "Base/Types/BasicTypes.h"

class GameplayTimestamp
{
public:
	// as 1/30000 of a second
	using TimeValueType = u64;

public:
	constexpr GameplayTimestamp() = default;
	explicit constexpr GameplayTimestamp(TimeValueType time) noexcept : mTimestamp(time) {}

	[[nodiscard]] bool isInitialized() const noexcept;

	[[nodiscard]] bool operator==(GameplayTimestamp other) const noexcept;
	[[nodiscard]] bool operator!=(GameplayTimestamp other) const noexcept;
	[[nodiscard]] bool operator<(GameplayTimestamp other) const noexcept;
	[[nodiscard]] bool operator<=(GameplayTimestamp other) const noexcept;
	[[nodiscard]] bool operator>(GameplayTimestamp other) const noexcept;
	[[nodiscard]] bool operator>=(GameplayTimestamp other) const noexcept;

	void increaseByFloatTime(float passedTime) noexcept;
	[[nodiscard]] GameplayTimestamp getIncreasedByFloatTime(float passedTime) const noexcept;
	[[nodiscard]] GameplayTimestamp getDecreasedByFloatTime(float timeInThePast) const noexcept;

	friend void to_json(nlohmann::json& outJson, const GameplayTimestamp timestamp);
	friend void from_json(const nlohmann::json& json, GameplayTimestamp& outTimestamp);

public:
	static constexpr float TimeMultiplier = 300.0f;

private:
	static constexpr TimeValueType UNINITIALIZED_TIME = std::numeric_limits<TimeValueType>::max();
	TimeValueType mTimestamp = UNINITIALIZED_TIME;
};
