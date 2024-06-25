#include "EngineCommon/precomp.h"

#include "GameData/GameData.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"

#include <nlohmann/json.hpp>

GameData::GameData(const ComponentFactory& componentFactory)
	: mGameComponents(componentFactory)
{}

nlohmann::json GameData::toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder) const
{
	return nlohmann::json{
		{"game_components", Json::SerializeComponentSetHolder(mGameComponents, jsonSerializerHolder)}
	};
}

void GameData::fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	Json::DeserializeComponentSetHolder(mGameComponents, json.at("game_components"), jsonSerializerHolder);
}
