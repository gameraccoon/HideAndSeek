#include "Base/precomp.h"

#include "GameData/Spatial/SpatialEntity.h"

#include <nlohmann/json.hpp>

SpatialEntity::SpatialEntity(Entity entity, CellPos cellPos)
	: entity(entity)
	, cell(cellPos)
{
}

bool SpatialEntity::operator==(const SpatialEntity& other) const
{
	return cell == other.cell && entity.getID() == other.entity.getID();
}

bool SpatialEntity::operator!=(const SpatialEntity& other) const
{
	return !(*this == other);
}

void to_json(nlohmann::json& outJson, const SpatialEntity& spatialEntity)
{
	outJson = nlohmann::json{
		{"cell", spatialEntity.cell},
		{"entity", spatialEntity.entity}
	};
}

void from_json(const nlohmann::json& json, SpatialEntity& outSpatialEntity)
{
	json.at("cell").get_to(outSpatialEntity.cell);
	json.at("entity").get_to(outSpatialEntity.entity);
}
