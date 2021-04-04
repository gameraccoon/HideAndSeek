#pragma once

#include "editorcommand.h"
#include "ECS/Entity.h"

#include <nlohmann/json.hpp>

class World;
struct ComponentSerializersHolder;

class RemoveEntityCommand : public EditorCommand
{
public:
	RemoveEntityCommand(Entity entity, const ComponentSerializersHolder& serializerHolder);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	Entity mEntity;
	nlohmann::json mSerializedComponents;
	const ComponentSerializersHolder& mComponentSerializerHolder;
};
