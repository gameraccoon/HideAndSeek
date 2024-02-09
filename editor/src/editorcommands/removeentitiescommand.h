#pragma once

#include "editorcommand.h"

#include "../editorutils/editorentityreference.h"

#include "GameData/Serialization/Json/JsonComponentSerializer.h"
#include "GameData/Spatial/SpatialEntity.h"

#include <nlohmann/json.hpp>

class World;

class RemoveEntitiesCommand : public EditorCommand
{
public:
	RemoveEntitiesCommand(const std::vector<EditorEntityReference>& entities, const Json::ComponentSerializationHolder& serializerHolder);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	const std::vector<EditorEntityReference> mEntities;
	const Json::ComponentSerializationHolder& mComponentSerializerHolder;
	std::vector<nlohmann::json> mSerializedComponents;
};
