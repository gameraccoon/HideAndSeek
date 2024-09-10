#pragma once

#include <string>

#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class World;

namespace Utils
{
	void SaveWorld(World& world, const std::string& fileName, const Json::ComponentSerializationHolder& jsonSerializerHolder);
}
