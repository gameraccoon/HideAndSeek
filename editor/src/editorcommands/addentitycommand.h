#pragma once

#include <optional>

#include <raccoon-ecs/entity.h>

#include "editorcommand.h"

class World;

class AddEntityCommand : public EditorCommand
{
public:
	AddEntityCommand();

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	std::optional<size_t> mEditorEntityId;
};
