#include "addspatialentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include <GameData/World.h>
#include <GameData/Components/TransformComponent.generated.h>

AddSpatialEntityCommand::AddSpatialEntityCommand(const SpatialEntity& entity, const Vector2D& location)
	: EditorCommand(EffectType::Entities)
	, mEntity(entity)
	, mLocation(location)
{
}

void AddSpatialEntityCommand::doCommand(World* world)
{
	WorldCell& cell = world->getSpatialData().getOrCreateCell(mEntity.cell);
	cell.getEntityManager().insertEntityUnsafe(mEntity.entity.getEntity());
	TransformComponent* transform = cell.getEntityManager().addComponent<TransformComponent>(mEntity.entity.getEntity());
	transform->setLocation(mLocation);
}

void AddSpatialEntityCommand::undoCommand(World* world)
{
	if (WorldCell* cell = world->getSpatialData().getCell(mEntity.cell))
	{
		cell->getEntityManager().removeEntity(mEntity.entity.getEntity());
	}
}
