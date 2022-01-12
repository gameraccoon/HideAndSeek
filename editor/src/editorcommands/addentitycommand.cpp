#include "addentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include "GameData/World.h"

AddEntityCommand::AddEntityCommand(Entity entity)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntity(entity)
{
}

void AddEntityCommand::doCommand(World* world)
{
	EntityManager& worldEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(world->getEntityManager());
	worldEntityManager.addExistingEntityUnsafe(mEntity);
}

void AddEntityCommand::undoCommand(World* world)
{
	EntityManager& worldEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(world->getEntityManager());
	worldEntityManager.removeEntity(mEntity);
}
