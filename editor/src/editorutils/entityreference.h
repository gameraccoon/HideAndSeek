#pragma once

#include <optional>

#include "GameData/Spatial/CellPos.h"
#include "GameData/Spatial/SpatialEntity.h"

struct EntityReference
{
	Entity entity;
	/** nullopt indicates that the component is not bound to a cell */
	std::optional<CellPos> cellPos;

	explicit EntityReference(Entity entity) : entity(entity) {}

	explicit EntityReference(SpatialEntity spatialEntity)
		: entity(spatialEntity.entity.getEntity())
		, cellPos(spatialEntity.cell)
	{
		Assert(spatialEntity.entity.isValid(), "Spatial entity should be valid to be converted to EntityReference correctly");
	}
};
