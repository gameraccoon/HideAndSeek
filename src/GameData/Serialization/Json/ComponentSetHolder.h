#pragma once

#include <nlohmann/json_fwd.hpp>

#include "GameData/EcsDefinitions.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

namespace Json
{
	nlohmann::json SerializeComponentSetHolder(const ComponentSetHolder& componentSetHolder, const ComponentSerializationHolder& jsonSerializerHolder);
	void DeserializeComponentSetHolder(ComponentSetHolder& outComponentSetHolder, const nlohmann::json& json, const ComponentSerializationHolder& jsonSerializerHolder);
}
