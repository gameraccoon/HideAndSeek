#pragma once

#include "editorcommand.h"
#include "src/editorutils/componentreference.h"
#include <nlohmann/json.hpp>

#include "GameData/EcsDefinitions.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class World;

class RemoveComponentCommand final : public EditorCommand
{
public:
	RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const Json::ComponentSerializationHolder& jsonSerializerHolder, const ComponentFactory& componentFactory);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	ComponentSourceReference mSource;
	StringId mComponentTypeName;
	const Json::ComponentSerializationHolder& mComponentSerializerHolder;
	const ComponentFactory& mComponentFactory;
	nlohmann::json mSerializedComponent;
};
