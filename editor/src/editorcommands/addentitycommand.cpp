#include "addentitycommand.h"

#include <iostream>

#include <QtWidgets/qcombobox.h>

#include "../editorutils/editoridutils.h"

#include "GameData/World.h"

AddEntityCommand::AddEntityCommand()
	: EditorCommand(EffectBitset(EffectType::Entities))
{
}

void AddEntityCommand::doCommand(CommandExecutionContext& context)
{
	if (!mEditorEntityId)
	{
		mEditorEntityId = context.getEditorIdGenerator().getNextId();
	}

	EntityManager& worldEntityManager = context.world->getEntityManager();
	const Entity entity = worldEntityManager.addEntity();
	Utils::SetEntityId(entity, *mEditorEntityId, worldEntityManager);
}

void AddEntityCommand::undoCommand(CommandExecutionContext& context)
{
	EntityManager& worldEntityManager = context.world->getEntityManager();
	const OptionalEntity entity = Utils::GetEntityFromId(*mEditorEntityId, worldEntityManager);
	if (!entity.isValid())
	{
		std::cout << "AddEntityCommand::undoCommand: entity with id " << *mEditorEntityId << " not found\n";
		return;
	}
	worldEntityManager.removeEntity(entity.getEntity());
}
