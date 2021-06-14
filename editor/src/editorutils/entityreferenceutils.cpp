#include "entityreferenceutils.h"

#include "GameData/World.h"

#include "src/EditorDataAccessor.h"

namespace Utils
{
	std::optional<EntityView> GetEntityView(const EntityReference& reference, World* world)
	{
		if (reference.cellPos.has_value())
		{
			if (WorldCell* cell = world->getSpatialData().getCell(*reference.cellPos)) // spatial entity
			{
				return EntityView(reference.entity, gEditorDataAccessor.getSingleThreadedEntityManager(cell->getEntityManager()));
			}
			else
			{
				return std::nullopt;
			}
		}
		else // world entity
		{
			return EntityView(reference.entity, gEditorDataAccessor.getSingleThreadedEntityManager(world->getEntityManager()));
		}
	}

	std::optional<EntityView> GetSpatialEntityView(const SpatialEntity& spatialEntity, World* world)
	{
		if (spatialEntity.isValid())
		{
			if (WorldCell* cell = world->getSpatialData().getCell(spatialEntity.cell))
			{
				return EntityView(spatialEntity.entity.getEntity(), gEditorDataAccessor.getSingleThreadedEntityManager(cell->getEntityManager()));
			}
		}
		return std::nullopt;
	}
}
