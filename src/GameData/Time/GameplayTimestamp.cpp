#include "EngineCommon/precomp.h"

#include "GameData/Time/GameplayTimestamp.h"

#include <type_traits>

#include <nlohmann/json.hpp>

bool GameplayTimestamp::isInitialized() const noexcept
{
	return mTimestamp != UNINITIALIZED_TIME;
}

void GameplayTimestamp::increaseByUpdateCount(s32 passedUpdates) noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	mTimestamp += passedUpdates * TimeMultiplier;
}

GameplayTimestamp GameplayTimestamp::getIncreasedByUpdateCount(s32 passedUpdates) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	return GameplayTimestamp(mTimestamp + passedUpdates * TimeMultiplier);
}

GameplayTimestamp GameplayTimestamp::getDecreasedByUpdateCount(s32 updatesAgo) const noexcept
{
	Assert(isInitialized(), "Timestamp should be initialized before being used");
	return GameplayTimestamp(mTimestamp - updatesAgo * TimeMultiplier);
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
static_assert(sizeof(GameplayTimestamp) == 4, "GameplayTimestamp changed its size, is this a mistake?");
