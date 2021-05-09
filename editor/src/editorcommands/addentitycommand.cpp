#include "addentitycommand.h"

#include <QtWidgets/qcombobox.h>

#include "Base/Debug/Assert.h"

#include "GameData/World.h"

AddEntityCommand::AddEntityCommand(Entity entity)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntity(entity)
{
}

void AddEntityCommand::doCommand(World* world)
{
	bool isSuccessful = world->getEntityManager().tryInsertEntity(mEntity);
	if (!isSuccessful)
	{
		ReportError("Collision when adding/readding an entity. State is corrupted, no recovery can be performed. Make backups before saving.");
	}
}

void AddEntityCommand::undoCommand(World* world)
{
	world->getEntityManager().removeEntity(mEntity);
}
