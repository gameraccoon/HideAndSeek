#pragma once

#include <optional>

#include "editorcommand.h"

class World;

class AddEntityCommand final : public EditorCommand
{
public:
	AddEntityCommand();

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	std::optional<size_t> mEditorEntityId;
};
