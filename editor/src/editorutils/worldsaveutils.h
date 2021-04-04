#pragma once

#include <string>

class World;
struct ComponentSerializersHolder;

namespace Utils
{
	void SaveWorld(World& world, const std::string& fileName, const ComponentSerializersHolder& serializationHolder);
}
