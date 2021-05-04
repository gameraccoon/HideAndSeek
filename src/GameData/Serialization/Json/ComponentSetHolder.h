#pragma once

#include <nlohmann/json_fwd.hpp>

#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "GameData/EcsDefinitions.h"

namespace Json
{
	nlohmann::json SerializeComponentSetHolder(const ComponentSetHolder& componentSetHolder, const Ecs::ComponentSerializersHolder& componentSerializers);
	void DeserializeComponentSetHolder(ComponentSetHolder& outComponentSetHolder, const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers);
}
