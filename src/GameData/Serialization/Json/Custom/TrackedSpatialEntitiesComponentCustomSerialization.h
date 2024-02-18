#pragma once

#include <map>

#include <nlohmann/json.hpp>

#include "Base/Types/String/StringId.h"

struct SpatialEntity;

namespace TrackedSpatialEntitiesComponentCustomSerialization
{
	nlohmann::json SerializeEntities(const std::map<StringId, SpatialEntity>& entities);
	void DeserializeEntities(const nlohmann::json& json, std::map<StringId, SpatialEntity>& outEntities);
};
