#pragma once

#include <vector>

#include "raccoon-ecs/async_operations.h"

#include "GameData/EcsDefinitions.h"

#include "GameData/Spatial/WorldCell.h"

class SpatialEntityManager
{
public:
	explicit SpatialEntityManager(const std::vector<WorldCell*>& cells);

	template<typename Operation, typename DataVector>
	void getComponents(const Operation& operation, DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template getComponents(cell->getEntityManager(), inOutComponents);
		}
	}

	template<typename Operation, typename DataVector>
	void getSpatialComponents(const Operation& operation, DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template getComponents(cell->getEntityManager(), inOutComponents, cell);
		}
	}

	template<typename Operation, typename DataVector>
	void getSpatialComponentsWithEntities(Operation& operation, DataVector& inOutComponents)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template getComponentsWithEntities(cell->getEntityManager(), inOutComponents, cell);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachComponentSet(const Operation& operation, FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template forEachComponentSet(cell->getEntityManager(), processor);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachSpatialComponentSet(const Operation& operation, FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template forEachComponentSet(cell->getEntityManager(), processor, cell);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(const Operation& operation, FunctionType processor)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template forEachComponentSetWithEntity(cell->getEntityManager(), processor, cell);
		}
	}

	template<typename Operation>
	void executeScheduledActions(const Operation& operation)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template executeScheduledActions(cell->getEntityManager());
		}
	}

	template<typename Operation>
	void getAllEntityComponents(const Operation& operation, Entity entity, std::vector<TypedComponent>& outComponents)
	{
		for (WorldCell* cell : mCells)
		{
			operation.template getAllEntityComponents(cell->getEntityManager(), entity, outComponents);

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

	template<typename Operation, typename DataVector>
	void getComponents(const Operation& operation, DataVector& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template getComponents(cell->getEntityManager(), inOutComponents);
		}
	}

	template<typename Operation, typename DataVector>
	void getSpatialComponents(const Operation& operation, DataVector& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template getComponents(cell->getEntityManager(), inOutComponents, cell);
		}
	}

	template<typename Operation, typename DataVector>
	void getSpatialComponentsWithEntities(Operation& operation, DataVector& inOutComponents) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template getComponentsWithEntities(cell->getEntityManager(), inOutComponents, cell);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachComponentSet(const Operation& operation, FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template forEachComponentSet(cell->getEntityManager(), processor);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachSpatialComponentSet(const Operation& operation, FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template forEachComponentSet(cell->getEntityManager(), processor, cell);
		}
	}

	template<typename Operation, typename FunctionType>
	void forEachSpatialComponentSetWithEntity(const Operation& operation, FunctionType processor) const
	{
		for (const WorldCell* cell : mCells)
		{
			operation.template forEachComponentSetWithEntity(cell->getEntityManager(), processor, cell);
		}
	}

private:
	std::vector<const WorldCell*> mCells;
};
