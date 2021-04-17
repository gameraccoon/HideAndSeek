#pragma once

#include <nlohmann/json_fwd.hpp>

#include "ECS/Entity.h"

#include "GameData/Spatial/CellPos.h"

struct SpatialEntity
{
	OptionalEntity entity;
	CellPos cell;

	SpatialEntity() = default;
	SpatialEntity(Entity entity, CellPos cellPos);

	bool isValid() const { return entity.isValid(); }

	bool operator==(const SpatialEntity& other) const;
	bool operator!=(const SpatialEntity& other) const;

	friend void to_json(nlohmann::json& outJson, const SpatialEntity& spatialEntity);
	friend void from_json(const nlohmann::json& json, SpatialEntity& outSpatialEntity);
};
