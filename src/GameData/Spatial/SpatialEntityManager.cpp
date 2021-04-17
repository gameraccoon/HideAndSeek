#include "Base/precomp.h"

#include "GameData/Spatial/SpatialEntityManager.h"

SpatialEntityManager::SpatialEntityManager(const std::vector<WorldCell*>& cells)
	: mCells(cells)
{
}

void SpatialEntityManager::executeScheduledActions()
{
	for (WorldCell* cell : mCells)
	{
		cell->getEntityManager().executeScheduledActions();
	}
}

void SpatialEntityManager::getAllEntityComponents(Entity entity, std::vector<BaseComponent*>& outComponents)
{
	for (WorldCell* cell : mCells)
	{
		cell->getEntityManager().getAllEntityComponents(entity, outComponents);

		if (!outComponents.empty())
		{
			break;
		}
	}
}

void SpatialEntityManager::getSpatialEntitiesHavingComponents(const std::vector<StringId>& componentIndexes, TupleVector<WorldCell*, Entity>& inOutEntities) const
{
	std::vector<Entity> entities;

	for (WorldCell* cell : mCells)
	{
		entities.clear();
		cell->getEntityManager().getEntitiesHavingComponents(componentIndexes, entities);
		std::transform(
			entities.begin(),
			entities.end(),
			std::back_inserter(inOutEntities),
			[cell](Entity entity)
			{
				return std::make_tuple(cell, entity);
			}
		);
	}
}

WorldCell* SpatialEntityManager::findEntityCell(Entity entity)
{
	for (WorldCell* cell : mCells)
	{
		if (cell->getEntityManager().hasEntity(entity))
		{
			return cell;
		}
	}
	return nullptr;
}

ConstSpatialEntityManager::ConstSpatialEntityManager(const std::vector<const WorldCell*>& cells)
	: mCells(cells)
{
}
