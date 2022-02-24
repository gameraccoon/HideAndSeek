#pragma once

#include <string>

#include "GameData/EcsDefinitions.h"

class World;
class GameData;


namespace Json {
	class ComponentSerializationHolder;
}

namespace GameDataLoader
{
	void SaveWorld(World& world, const std::string& levelName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void LoadWorld(World& world, const std::string& levelName, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	void SaveGameData(const GameData& gameData, const std::string& gameDataName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void LoadGameData(GameData& gameData, const std::string& gameDataName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
}
