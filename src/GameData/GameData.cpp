#include "Base/precomp.h"

#include "GameData/GameData.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"

#include <nlohmann/json.hpp>

GameData::GameData(const ComponentFactory& componentFactory)
	: mGameComponents(componentFactory)
{}

nlohmann::json GameData::toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const
{
	return nlohmann::json{
		{"game_components", Json::SerializeComponentSetHolder(mGameComponents, componentSerializers)}
	};
}

void GameData::fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers)
{
	Json::DeserializeComponentSetHolder(mGameComponents, json.at("game_components"), componentSerializers);
}
