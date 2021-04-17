#include "changeentitygrouplocationcommand.h"

#include <GameData/World.h>
#include <GameData/Components/TransformComponent.generated.h>

ChangeEntityGroupLocationCommand::ChangeEntityGroupLocationCommand(const std::vector<SpatialEntity>& entities, Vector2D shift)
	: EditorCommand(EffectBitset(EffectType::ComponentAttributes, EffectType::EntityLocations))
	, mShift(shift)
	, mOriginalEntities(entities)
{
}

void ChangeEntityGroupLocationCommand::doCommand(World* world)
{
	mOriginalEntitiesPos.resize(mOriginalEntities.size());
	mModifiedEntities.resize(mOriginalEntities.size());
	mModifiedEntitiesPos.resize(mOriginalEntities.size());

	for (size_t i = 0; i < mOriginalEntities.size(); ++i)
	{
		const SpatialEntity& spatialEntity = mOriginalEntities[i];
		if (WorldCell* cell = world->getSpatialData().getCell(spatialEntity.cell))
		{
			auto [component] = cell->getEntityManager().getEntityComponents<TransformComponent>(spatialEntity.entity.getEntity());
			if (component)
			{
				const Vector2D originalPos = component->getLocation();
				mOriginalEntitiesPos[i] = originalPos;
				const Vector2D newPos = originalPos + mShift;
				mModifiedEntitiesPos[i] = newPos;
				const CellPos newCellPos = SpatialWorldData::GetCellForPos(newPos);
				component->setLocation(newPos);
				mModifiedEntities[i].cell = newCellPos;
				mModifiedEntities[i].entity = spatialEntity.entity;

				if (newCellPos != spatialEntity.cell)
				{
					WorldCell& newCell = world->getSpatialData().getOrCreateCell(newCellPos);
					cell->getEntityManager().transferEntityTo(newCell.getEntityManager(), spatialEntity.entity.getEntity());
				}
			}
		}
	}
}

void ChangeEntityGroupLocationCommand::undoCommand(World* world)
{
	for (size_t i = 0; i < mModifiedEntities.size(); ++i)
	{
		const SpatialEntity& spatialEntity = mModifiedEntities[i];
		if (WorldCell* cell = world->getSpatialData().getCell(spatialEntity.cell))
		{
			auto [component] = cell->getEntityManager().getEntityComponents<TransformComponent>(spatialEntity.entity.getEntity());
			if (component)
			{
				const Vector2D newLocation = mOriginalEntitiesPos[i];
				const CellPos newCellPos = SpatialWorldData::GetCellForPos(newLocation);
				component->setLocation(newLocation);

				if (newCellPos != spatialEntity.cell)
				{
					Assert(mOriginalEntities[i].cell == newCellPos, "Inconsistent undo/redo transformations in ChangeEntityGroupLocationCommand");
					WorldCell& newCell = world->getSpatialData().getOrCreateCell(newCellPos);
					cell->getEntityManager().transferEntityTo(newCell.getEntityManager(), spatialEntity.entity.getEntity());
				}
			}
		}
	}
}
