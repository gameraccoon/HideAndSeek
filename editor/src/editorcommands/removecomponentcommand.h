#pragma once

#include "editorcommand.h"

#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "GameData/EcsDefinitions.h"

#include <nlohmann/json.hpp>

#include "src/editorutils/componentreference.h"

class World;

class RemoveComponentCommand : public EditorCommand
{
public:
	RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const Ecs::ComponentSerializersHolder& serializerHolder, const ComponentFactory& componentFactory);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	ComponentSourceReference mSource;
	StringId mComponentTypeName;
	const Ecs::ComponentSerializersHolder& mComponentSerializerHolder;
	const ComponentFactory& mComponentFactory;
	nlohmann::json mSerializedComponent;
};
