#include "addspatialentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include <GameData/World.h>
#include <GameData/Components/TransformComponent.generated.h>

AddSpatialEntityCommand::AddSpatialEntityCommand(const SpatialEntity& entity, const Vector2D& location)
	: mEntity(entity)
	, mLocation(location)
{
}

bool AddSpatialEntityCommand::doCommand(World* world)
{
	WorldCell& cell = world->getSpatialData().getOrCreateCell(mEntity.cell);
	cell.getEntityManager().insertEntityUnsafe(mEntity.entity.getEntity());
	TransformComponent* transform = cell.getEntityManager().addComponent<TransformComponent>(mEntity.entity.getEntity());
	transform->setLocation(mLocation);
	return false;
}

bool AddSpatialEntityCommand::undoCommand(World* world)
{
	if (WorldCell* cell = world->getSpatialData().getCell(mEntity.cell))
	{
		cell->getEntityManager().removeEntity(mEntity.entity.getEntity());
	}
	return false;
}

EditorCommand::EffectType AddSpatialEntityCommand::getEffectType()
{
	return EffectType::Entities;
}
