#include "removeentitiescommand.h"

#include <QtWidgets/qcombobox.h>

#include "GameData/World.h"
#include "GameData/Serialization/Json/EntityManager.h"

#include "src/EditorDataAccessor.h"

RemoveEntitiesCommand::RemoveEntitiesCommand(const std::vector<SpatialEntity>& entities, const Json::ComponentSerializationHolder& jsonSerializerHolder)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntities(entities)
	, mComponentSerializerHolder(jsonSerializerHolder)
{
}

void RemoveEntitiesCommand::doCommand(World* world)
{
	if (mSerializedComponents.empty())
	{
		mSerializedComponents.resize(mEntities.size());

		for (size_t i = 0, iSize = mEntities.size(); i < iSize; ++i)
		{
			WorldCell* cell = world->getSpatialData().getCell(mEntities[i].cell);
			if (cell != nullptr)
			{
				EntityManager& cellEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(cell->getEntityManager());
				Json::GetPrefabFromEntity(cellEntityManager, mSerializedComponents[i], mEntities[i].entity.getEntity(), mComponentSerializerHolder);
			}
		}
	}

	for (const SpatialEntity& entity : mEntities)
	{
		WorldCell* cell = world->getSpatialData().getCell(entity.cell);
		if (cell != nullptr)
		{
			EntityManager& cellEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(cell->getEntityManager());
			cellEntityManager.removeEntity(entity.entity.getEntity());
		}
	}
}

void RemoveEntitiesCommand::undoCommand(World* world)
{
	for (size_t i = 0, iSize = mEntities.size(); i < iSize; ++i)
	{
		WorldCell& cell = world->getSpatialData().getOrCreateCell(mEntities[i].cell);
		EntityManager& cellEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(cell.getEntityManager());
		cellEntityManager.reinsertPrevioslyExistingEntity(mEntities[i].entity.getEntity());
		Json::ApplyPrefabToExistentEntity(cellEntityManager, mSerializedComponents[i], mEntities[i].entity.getEntity(), mComponentSerializerHolder);
	}
}
