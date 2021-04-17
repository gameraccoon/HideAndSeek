#pragma once

#include "editorcommand.h"

#include "ECS/Entity.h"

#include <nlohmann/json.hpp>

#include "src/editorutils/componentreference.h"

class World;
struct ComponentSerializersHolder;

class RemoveComponentCommand : public EditorCommand
{
public:
	RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const ComponentSerializersHolder& serializerHolder);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	ComponentSourceReference mSource;
	StringId mComponentTypeName;
	const ComponentSerializersHolder& mComponentSerializerHolder;
	nlohmann::json mSerializedComponent;
};
