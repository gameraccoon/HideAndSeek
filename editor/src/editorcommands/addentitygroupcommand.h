#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "editorcommand.h"

#include "GameData/Serialization/Json/JsonComponentSerializer.h"
#include "GameData/Geometry/Vector2D.h"
#include "GameData/Spatial/SpatialEntity.h"

class World;

class AddEntityGroupCommand : public EditorCommand
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
