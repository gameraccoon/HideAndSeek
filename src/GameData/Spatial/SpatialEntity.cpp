#include "Base/precomp.h"

#include "GameData/Spatial/SpatialEntity.h"

SpatialEntity::SpatialEntity(Entity entity, CellPos cellPos)
	: entity(entity)
	, cell(cellPos)
{
}

bool SpatialEntity::operator==(const SpatialEntity& other) const
{
	return cell == other.cell
		&& entity.isValid() && other.entity.isValid()
		&& entity.getEntity() == other.entity.getEntity();
}

bool SpatialEntity::operator!=(const SpatialEntity& other) const
{
	return !(*this == other);
}
