#include "addentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include <GameData/World.h>

AddEntityCommand::AddEntityCommand(Entity entity)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntity(entity)
{
}

void AddEntityCommand::doCommand(World* world)
{
	world->getEntityManager().insertEntityUnsafe(mEntity);
}

void AddEntityCommand::undoCommand(World* world)
{
	world->getEntityManager().removeEntity(mEntity);
}
