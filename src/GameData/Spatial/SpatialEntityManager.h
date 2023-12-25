#pragma once

#include <raccoon-ecs/utils/combined_entity_manager_view.h>

#include "GameData/EcsDefinitions.h"

class WorldCell;

using SpatialEntityManager = RaccoonEcs::CombinedEntityManagerView<EntityManager, std::reference_wrapper<WorldCell>>;
using ConstSpatialEntityManager = RaccoonEcs::CombinedEntityManagerView<const EntityManager, std::reference_wrapper<const WorldCell>>;
