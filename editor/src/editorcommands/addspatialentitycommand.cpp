#include "addspatialentitycommand.h"

#include <QtWidgets/qcombobox.h>

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
	EntityManager& cellEnttiyManager = cell.getEntityManager();
	cellEnttiyManager.addExistingEntityUnsafe(mEntity.entity.getEntity());
	TransformComponent* transform = cellEnttiyManager.addComponent<TransformComponent>(mEntity.entity.getEntity());
	transform->setLocation(mLocation);
}

void AddSpatialEntityCommand::undoCommand(World* world)
{
	if (WorldCell* cell = world->getSpatialData().getCell(mEntity.cell))
	{
		EntityManager& cellEnttiyManager = cell->getEntityManager();
		cellEnttiyManager.removeEntity(mEntity.entity.getEntity());
	}
}
