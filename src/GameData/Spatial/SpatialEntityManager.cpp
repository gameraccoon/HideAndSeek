#include "Base/precomp.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/Spatial/SpatialEntityManager.h"

SpatialEntityManager::SpatialEntityManager(const std::vector<WorldCell*>& cells)
	: mCells(cells)
{
}

ConstSpatialEntityManager::ConstSpatialEntityManager(const std::vector<const WorldCell*>& cells)
	: mCells(cells)
{
}
