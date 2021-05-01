#pragma once

#include <string>

#include "GameData/EcsDefinitions.h"

class World;
class GameData;

namespace GameDataLoader
{
	void SaveWorld(const World& world, const std::string& levelName, const Ecs::ComponentSerializersHolder& componentSerializers);
	void LoadWorld(World& world, const std::string& levelName, const Ecs::ComponentSerializersHolder& componentSerializers);

	void SaveGameData(const GameData& gameData, const std::string& gameDataName, const Ecs::ComponentSerializersHolder& componentSerializers);
	void LoadGameData(GameData& gameData, const std::string& gameDataName, const Ecs::ComponentSerializersHolder& componentSerializers);
}
