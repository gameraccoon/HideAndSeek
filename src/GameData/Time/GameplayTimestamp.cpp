#include "Base/precomp.h"

#include "GameData/Time/GameplayTimestamp.h"

#include <type_traits>
#include <nlohmann/json.hpp>

static auto PositiveRoundFunc(float toRound)
{
	using namespace std; // to compile on both msvc and clang/gcc
	Assert(toRound >= 0.0f, "PositiveRoundFunc works only with positive values");
	return floor(toRound - 0.5);
}

GameplayTimestamp::TimeValueType GetConvertedPassedTime(float passedTime)
{
	return static_cast<GameplayTimestamp::TimeValueType>(PositiveRoundFunc(passedTime * GameplayTimestamp::TimeMultiplier));
}

bool GameplayTimestamp::isInitialized() const noexcept
{
	return mTimestamp != UNINITIALIZED_TIME;
}

bool GameplayTimestamp::operator==(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp == other.mTimestamp;
}

bool GameplayTimestamp::operator!=(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp != other.mTimestamp;
}

bool GameplayTimestamp::operator<(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp < other.mTimestamp;
}

bool GameplayTimestamp::operator<=(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp <= other.mTimestamp;
}

bool GameplayTimestamp::operator>(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp > other.mTimestamp;
}

bool GameplayTimestamp::operator>=(GameplayTimestamp other) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	Assert(other.isInitialized(), "Timestamp should be initialized before being used");
	return mTimestamp >= other.mTimestamp;
}

void GameplayTimestamp::increaseByFloatTime(float passedTime) noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	mTimestamp += GetConvertedPassedTime(passedTime);
}

GameplayTimestamp GameplayTimestamp::getIncreasedByFloatTime(float passedTime) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	return GameplayTimestamp(mTimestamp + GetConvertedPassedTime(passedTime));
}

void to_json(nlohmann::json& outJson, const GameplayTimestamp timestamp)
{
	outJson = nlohmann::json::object({
		{"value", timestamp.mTimestamp}
	});
}

void from_json(const nlohmann::json& json, GameplayTimestamp& outTimestamp)
{
	json.at("value").get_to(outTimestamp.mTimestamp);
}

static_assert(std::is_trivially_copyable<GameplayTimestamp>(), "GameplayTimestamp should be trivially copyable");
