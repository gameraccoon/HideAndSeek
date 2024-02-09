#pragma once

#include <map>

#include <nlohmann/json_fwd.hpp>

#include "Base/Types/String/StringId.h"

class SpatialEntity;

namespace TrackedSpatialEntitiesComponentCustomSerialization
{
	nlohmann::json SerializeEntities(const std::map<StringId, SpatialEntity>& entities);
	void DeserializeEntities(const nlohmann::json& json, std::map<StringId, SpatialEntity>& outEntities);
};
