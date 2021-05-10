#pragma once

#include <raccoon-ecs/entity.h>
#include <raccoon-ecs/component_factory.h>

#include "editorcommand.h"

#include "src/editorutils/componentreference.h"

class World;

class AddComponentCommand : public EditorCommand
{
public:
	AddComponentCommand(const ComponentSourceReference& source, StringId typeName, const ComponentFactory& factory);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	ComponentSourceReference mSource;
	StringId mComponentTypeName;
	const ComponentFactory& mComponentFactory;
};
