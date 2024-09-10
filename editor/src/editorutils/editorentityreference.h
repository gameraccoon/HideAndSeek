#pragma once

#include <optional>

#include "GameData/Spatial/CellPos.h"

struct EditorEntityReference
{
	size_t editorUniqueId = 0;
	/** nullopt indicates that the component is bound to game-global entity manager */
	std::optional<CellPos> cellPos;

	explicit EditorEntityReference(const size_t editorId)
		: editorUniqueId(editorId) {}

	explicit EditorEntityReference(const size_t editorId, const CellPos cellPos)
		: editorUniqueId(editorId)
		, cellPos(cellPos)
	{}

	bool operator==(const EditorEntityReference& other) const
	{
		return editorUniqueId == other.editorUniqueId;
	}
};
