#include "removeentitiescommand.h"

#include <iostream>

#include <QtWidgets/qcombobox.h>

#include "../editorutils/editoridutils.h"

#include "GameData/World.h"
#include "GameData/Serialization/Json/EntityManager.h"

RemoveEntitiesCommand::RemoveEntitiesCommand(const std::vector<EditorEntityReference>& entities, const Json::ComponentSerializationHolder& jsonSerializerHolder)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntities(entities)
	, mComponentSerializerHolder(jsonSerializerHolder)
{
}

void RemoveEntitiesCommand::doCommand(CommandExecutionContext& context)
{
	if (mSerializedComponents.empty())
	{
		mSerializedComponents.resize(mEntities.size());

		for (size_t i = 0, iSize = mEntities.size(); i < iSize; ++i)
		{
			if (!mEntities[i].cellPos.has_value())
			{
				std::cout << "RemoveEntitiesCommand::doCommand: Entity " << mEntities[i].editorUniqueId << " has no cellPos\n";
				continue;
			}

			WorldCell* cell = context.world->getSpatialData().getCell(*mEntities[i].cellPos);
			if (cell == nullptr)
			{
				std::cout << "RemoveEntitiesCommand::doCommand: Could not find cell at " << mEntities[i].cellPos->x << ", " << mEntities[i].cellPos->y << "\n";
			}

			EntityManager& cellEntityManager = cell->getEntityManager();
			const OptionalEntity entity = Utils::GetEntityFromId(mEntities[i].editorUniqueId, cellEntityManager);
			if (!entity.isValid())
			{
				std::cout << "RemoveEntitiesCommand::doCommand: Could not find entity with id " << mEntities[i].editorUniqueId << "\n";
				continue;
			}
			Json::GetPrefabFromEntity(cellEntityManager, mSerializedComponents[i], entity.getEntity(), mComponentSerializerHolder);
		}
	}

	for (const EditorEntityReference& entityRef : mEntities)
	{
		if (!entityRef.cellPos.has_value())
		{
			std::cout << "RemoveEntitiesCommand::doCommand: Entity " << entityRef.editorUniqueId << " has no cellPos\n";
			continue;
		}

		WorldCell* cell = context.world->getSpatialData().getCell(*entityRef.cellPos);
		if (cell == nullptr)
		{
			std::cout << "RemoveEntitiesCommand::doCommand: Could not find cell at " << entityRef.cellPos->x << ", " << entityRef.cellPos->y << "\n";
			continue;
		}

		EntityManager& cellEntityManager = cell->getEntityManager();

		const OptionalEntity entity = Utils::GetEntityFromId(entityRef.editorUniqueId, cellEntityManager);
		if (!entity.isValid())
		{
			std::cout << "RemoveEntitiesCommand::doCommand: Could not find entity with id " << entityRef.editorUniqueId << "\n";
			continue;
		}

		cellEntityManager.removeEntity(entity.getEntity());
	}
}

void RemoveEntitiesCommand::undoCommand(CommandExecutionContext& context)
{
	for (size_t i = 0, iSize = mEntities.size(); i < iSize; ++i)
	{
		if (!mEntities[i].cellPos.has_value())
		{
			std::cout << "RemoveEntitiesCommand::undoCommand: Entity " << mEntities[i].editorUniqueId << " has no cellPos\n";
			continue;
		}

		WorldCell& cell = context.world->getSpatialData().getOrCreateCell(*mEntities[i].cellPos);
		EntityManager& cellEntityManager = cell.getEntityManager();

		Entity recreatedEntity = cellEntityManager.addEntity();
		Utils::SetEntityId(recreatedEntity, mEntities[i].editorUniqueId, cellEntityManager);

		Json::ApplyPrefabToExistentEntity(cellEntityManager, mSerializedComponents[i], recreatedEntity, mComponentSerializerHolder);
	}
}
