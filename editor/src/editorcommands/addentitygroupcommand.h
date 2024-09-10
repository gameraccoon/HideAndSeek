#pragma once

#include <vector>

#include "editorcommand.h"
#include <nlohmann/json.hpp>

#include "EngineData/Geometry/Vector2D.h"

#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class World;

class AddEntityGroupCommand final : public EditorCommand
{
public:
	AddEntityGroupCommand(const std::vector<nlohmann::json>& entities, const Json::ComponentSerializationHolder& jsonSerializerHolder, const Vector2D& shift);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	std::vector<nlohmann::json> mEntities;
	std::vector<EditorEntityReference> mCreatedEntities;
	std::vector<Vector2D> mCreatedEntityPositions;
	const Json::ComponentSerializationHolder& mSerializationHolder;
	Vector2D mShift;
};
