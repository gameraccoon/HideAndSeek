#include "removeentitiescommand.h"

#include <QtWidgets/qcombobox.h>

#include "ECS/Serialization/ComponentSerializersHolder.h"

#include "GameData/World.h"

RemoveEntitiesCommand::RemoveEntitiesCommand(const std::vector<SpatialEntity>& entities, const ComponentSerializersHolder& serializerHolder)
	: EditorCommand(EffectType::Entities)
	, mEntities(entities)
	, mComponentSerializerHolder(serializerHolder)
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
				cell->getEntityManager().getPrefabFromEntity(mSerializedComponents[i], mEntities[i].entity.getEntity(), mComponentSerializerHolder.jsonSerializer);
			}
		}
	}

	for (const SpatialEntity& entity : mEntities)
	{
		WorldCell* cell = world->getSpatialData().getCell(entity.cell);
		if (cell != nullptr)
		{
			cell->getEntityManager().removeEntity(entity.entity.getEntity());
		}
	}
}

void RemoveEntitiesCommand::undoCommand(World* world)
{
	for (size_t i = 0, iSize = mEntities.size(); i < iSize; ++i)
	{
		WorldCell& cell = world->getSpatialData().getOrCreateCell(mEntities[i].cell);
		cell.getEntityManager().insertEntityUnsafe(mEntities[i].entity.getEntity());
		cell.getEntityManager().applyPrefabToExistentEntity(mSerializedComponents[i], mEntities[i].entity.getEntity(), mComponentSerializerHolder);
	}
}
