#include "addspatialentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include "Base/Debug/Assert.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

AddSpatialEntityCommand::AddSpatialEntityCommand(const SpatialEntity& entity, const Vector2D& location)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntity(entity)
	, mLocation(location)
{
}

void AddSpatialEntityCommand::doCommand(World* world)
{
	WorldCell& cell = world->getSpatialData().getOrCreateCell(mEntity.cell);
	bool hasInserted = cell.getEntityManager().tryInsertEntity(mEntity.entity.getEntity());
	if (hasInserted)
	{
		TransformComponent* transform = cell.getEntityManager().addComponent<TransformComponent>(mEntity.entity.getEntity());
		transform->setLocation(mLocation);
	}
	else
	{
		ReportError("Entity can't be created because of ID collision. Can't recover. Back up your data before saving.");
	}
}

void AddSpatialEntityCommand::undoCommand(World* world)
{
	if (WorldCell* cell = world->getSpatialData().getCell(mEntity.cell))
	{
		cell->getEntityManager().removeEntity(mEntity.entity.getEntity());
	}
}
