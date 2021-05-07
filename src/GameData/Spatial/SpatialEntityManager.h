#pragma once

#include <vector>

#include "GameData/EcsDefinitions.h"

#include "GameData/Spatial/WorldCell.h"

class SpatialEntityManager
{
public:
	explicit SpatialEntityManager(const std::vector<WorldCell*>& cells);

	template<typename FirstComponent, typename... Components>
	void getComponents(std::vector<std::tuple<FirstComponent*, Components*...>>& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<FirstComponent, Components...>(inOutComponents);
		}
	}

	template<typename FirstComponent, typename... Components>
	void getSpatialComponents(std::vector<std::tuple<WorldCell*, FirstComponent*, Components*...>>& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<FirstComponent, Components...>(inOutComponents, cell);
		}
	}

	template<typename FirstComponent, typename... Components>
	void getSpatialComponentsWithEntities(std::vector<std::tuple<Entity, WorldCell*, FirstComponent*, Components*...>>& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponentsWithEntities<FirstComponent, Components...>(inOutComponents, cell);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachComponentSet(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<FirstComponent, Components...>(processor);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachSpatialComponentSet(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<FirstComponent, Components...>(processor, cell);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSetWithEntity<FirstComponent, Components...>(processor, cell);
		}
	}

	void executeScheduledActions();

	void getAllEntityComponents(Entity entity, std::vector<TypedComponent>& outComponents);

	void getSpatialEntitiesHavingComponents(const std::vector<StringId>& componentIndexes, std::vector<std::tuple<WorldCell*, Entity>>& inOutEntities) const;

	// debug function for imgui
	WorldCell* findEntityCell(Entity entity);

private:
	std::vector<WorldCell*> mCells;
};

class ConstSpatialEntityManager
{
public:
	explicit ConstSpatialEntityManager(const std::vector<const WorldCell*>& cells);

	template<typename FirstComponent, typename... Components>
	void getComponents(std::vector<std::tuple<FirstComponent*, Components*...>>& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<const FirstComponent, const Components...>(inOutComponents);
		}
	}

	template<typename FirstComponent, typename... Components>
	void getSpatialComponents(std::vector<std::tuple<const WorldCell*, const FirstComponent*, const Components*...>>& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<const FirstComponent, const Components...>(inOutComponents, cell);
		}
	}

	template<typename FirstComponent, typename... Components>
	void getSpatialComponentsWithEntities(std::vector<std::tuple<Entity, const WorldCell*, const FirstComponent*, const Components*...>>& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponentsWithEntities<const FirstComponent, const Components...>(inOutComponents, cell);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachComponentSet(FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<const FirstComponent, const Components...>(processor);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachSpatialComponentSet(FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<const FirstComponent, const Components...>(processor, cell);
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSetWithEntity<const FirstComponent, const Components...>(processor, cell);
		}
	}

private:
	std::vector<const WorldCell*> mCells;
};
