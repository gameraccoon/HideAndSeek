#pragma once

#include <raccoon-ecs/utils/entity_view.h>

#include "GameData/Spatial/SpatialEntity.h"

#include "entityreference.h"

class World;

namespace Utils
{
	using EntityView = RaccoonEcs::EntityViewImpl<StringId>;
	std::optional<EntityView> GetEntityView(const EntityReference& reference, World* world);
	std::optional<EntityView> GetSpatialEntityView(const SpatialEntity& spatialEntity, World* world);
}
