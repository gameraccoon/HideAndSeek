#pragma once

#include <raccoon-ecs/entity.h>

#include "editorcommand.h"

class World;

class AddEntityCommand : public EditorCommand
{
public:
	AddEntityCommand(Entity entity);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	Entity mEntity;
};
