#pragma once

#include <string>

#include "GameData/EcsDefinitions.h"

class World;
class GameData;

namespace RaccoonEcs
{
	class InnerDataAccessor;
}

namespace GameDataLoader
{
	using DataAccessor = const RaccoonEcs::InnerDataAccessor;

	void SaveWorld(World& world, DataAccessor& dataAccessor, const std::string& levelName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void LoadWorld(World& world, DataAccessor& dataAccessor, const std::string& levelName, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	void SaveGameData(const GameData& gameData, const std::string& gameDataName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void LoadGameData(GameData& gameData, const std::string& gameDataName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
}
