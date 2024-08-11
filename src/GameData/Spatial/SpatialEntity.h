#pragma once

#include "GameData/EcsDefinitions.h"
#include "GameData/Spatial/CellPos.h"

struct SpatialEntity
{
	OptionalEntity entity;
	CellPos cell = {};

	SpatialEntity() = default;
	SpatialEntity(Entity entity, CellPos cellPos);

	bool isValid() const { return entity.isValid(); }

	bool operator==(const SpatialEntity& other) const;
	bool operator!=(const SpatialEntity& other) const;
};
