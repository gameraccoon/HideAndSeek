#pragma once

#include <optional>

#include "editorcommand.h"

#include "EngineData/Geometry/Vector2D.h"

class World;

class AddSpatialEntityCommand final : public EditorCommand
{
public:
	AddSpatialEntityCommand(CellPos cellPos, const Vector2D& location);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	const CellPos mCellPos;
	const Vector2D mLocation;
	std::optional<size_t> mEditorEntityId;
};
