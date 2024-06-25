#pragma once

#include <limits>
#include <nlohmann/json_fwd.hpp>

#include "EngineCommon/Types/BasicTypes.h"

class GameplayTimestamp
{
public:
	using TimeValueType = u32;

public:
	constexpr GameplayTimestamp() = default;
	explicit constexpr GameplayTimestamp(TimeValueType time) noexcept : mTimestamp(time) {}

	[[nodiscard]] bool isInitialized() const noexcept;

	auto operator<=>(const GameplayTimestamp& other) const noexcept = default;

	void increaseByUpdateCount(s32 passedUpdates) noexcept;
	[[nodiscard]] GameplayTimestamp getIncreasedByUpdateCount(s32 passedUpdates) const noexcept;
	[[nodiscard]] GameplayTimestamp getDecreasedByUpdateCount(s32 updatesAgo) const noexcept;

	friend void to_json(nlohmann::json& outJson, GameplayTimestamp timestamp);
	friend void from_json(const nlohmann::json& json, GameplayTimestamp& outTimestamp);

	[[nodiscard]] TimeValueType getRawValue() const { return mTimestamp; }

public:
	static constexpr s32 TimeMultiplier = 1;

private:
	static constexpr TimeValueType UNINITIALIZED_TIME = std::numeric_limits<TimeValueType>::max();
	TimeValueType mTimestamp = UNINITIALIZED_TIME;
};
