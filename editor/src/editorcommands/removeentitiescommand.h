#pragma once

#include "../editorutils/editorentityreference.h"
#include "editorcommand.h"
#include <nlohmann/json.hpp>

#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class World;

class RemoveEntitiesCommand final : public EditorCommand
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
