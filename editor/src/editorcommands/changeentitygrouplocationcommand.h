#pragma once

#include "editorcommand.h"

#include "EngineData/Geometry/Vector2D.h"

#include "GameData/Spatial/CellPos.h"

class ChangeEntityGroupLocationCommand final : public EditorCommand
{
public:
	ChangeEntityGroupLocationCommand(const std::vector<EditorEntityReference>& entities, Vector2D shift);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

	const std::vector<size_t>& getEditorEntityReferences() const { return mEditorEntityReferences; }
	const std::vector<CellPos>& getOriginalCellsOfEntities() const { return mOriginalCellsOfEntities; }
	const std::vector<CellPos>& getModifiedCellsOfEntities() const { return mModifiedCellsOfEntities; }

private:
	const Vector2D mShift;
	std::vector<size_t> mEditorEntityReferences;
	std::vector<CellPos> mOriginalCellsOfEntities;
	std::vector<Vector2D> mOriginalPositionsOfEntities;

	std::vector<CellPos> mModifiedCellsOfEntities;
	std::vector<Vector2D> mModifiedPositionsOfEntities;
};
