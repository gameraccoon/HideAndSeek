#include "EngineCommon/precomp.h"

#include "TrackedSpatialEntitiesComponentCustomSerialization.h"

#include "GameData/Spatial/SpatialEntity.h"

namespace TrackedSpatialEntitiesComponentCustomSerialization
{
	nlohmann::json SerializeEntities(const std::map<StringId, SpatialEntity>& entities)
	{
		nlohmann::json json = nlohmann::json::array();
		for (const auto& entity : entities)
		{
			json.push_back(entity.first);
		}
		return json;
	}

	void DeserializeEntities(const nlohmann::json& json, std::map<StringId, SpatialEntity>& outEntities)
	{
		outEntities.clear();
		for (const auto& entity : json)
		{
			outEntities[entity.get<StringId>()] = SpatialEntity();
		}
	}
} // namespace TrackedSpatialEntitiesComponentCustomSerialization
