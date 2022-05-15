#pragma once

#include <vector>

#include "GameData/EcsDefinitions.h"

#include "GameData/Spatial/WorldCell.h"

class SpatialEntityManager
{
public:
	explicit SpatialEntityManager(const std::vector<WorldCell*>& cells);

	template<typename... Components, typename DataVector>
	void getComponents(DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<Components...>(inOutComponents);
		}
	}

	template<typename... Components, typename DataVector>
	void getSpatialComponents(DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<Components...>(inOutComponents, cell);
		}
	}

	template<typename... Components, typename DataVector>
	void getSpatialComponentsWithEntities(DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponentsWithEntities<Components...>(inOutComponents, cell);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachComponentSet(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<Components...>(processor);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachSpatialComponentSet(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<Components...>(processor, cell);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSetWithEntity<Components...>(processor, cell);
		}
	}

	void executeScheduledActions()
	{
		for (WorldCell* cell : mCells)
		{
			cell->getEntityManager().executeScheduledActions();
		}
	}

	void getAllEntityComponents(Entity entity, std::vector<TypedComponent>& outComponents)
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

private:
	std::vector<WorldCell*> mCells;
};

class ConstSpatialEntityManager
{
public:
	explicit ConstSpatialEntityManager(const std::vector<const WorldCell*>& cells);

	template<typename... Components, typename DataVector>
	void getComponents(DataVector& inOutComponents)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<Components...>(inOutComponents);
		}
	}

	template<typename... Components, typename DataVector>
	void getSpatialComponents(DataVector& inOutComponents)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponents<Components...>(inOutComponents, cell);
		}
	}

	template<typename... Components, typename DataVector>
	void getSpatialComponentsWithEntities(DataVector& inOutComponents)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getComponentsWithEntities<Components...>(inOutComponents, cell);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachComponentSet(FunctionType processor)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<Components...>(processor);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachSpatialComponentSet(FunctionType processor)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSet<Components...>(processor, cell);
		}
	}

	template<typename... Components, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(FunctionType processor)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().forEachComponentSetWithEntity<Components...>(processor, cell);
		}
	}

	void getAllEntityComponents(Entity entity, std::vector<ConstTypedComponent>& outComponents)
	{
		for (const WorldCell* cell : mCells)
		{
			cell->getEntityManager().getAllEntityComponents(entity, outComponents);

			if (!outComponents.empty())
			{
				break;
			}
		}
	}

private:
	std::vector<const WorldCell*> mCells;
};
