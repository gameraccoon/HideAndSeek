#include "Base/precomp.h"

#include "GameData/GameData.h"

#include <nlohmann/json.hpp>

GameData::GameData(const ComponentFactory& componentFactory)
	: mGameComponents(componentFactory)
{}

nlohmann::json GameData::toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const
{
	return nlohmann::json{
		{"game_components", mGameComponents.toJson(componentSerializers)}
	};
}

void GameData::fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers)
{
	mGameComponents.fromJson(json.at("game_components"), componentSerializers);
}
