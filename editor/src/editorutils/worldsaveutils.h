#pragma once

#include <string>
 #include "ECS/Serialization/ComponentSerializersHolder.h"

class World;

namespace Utils
{
	void SaveWorld(World& world, const std::string& fileName, const Ecs::ComponentSerializersHolder& serializationHolder);
}
