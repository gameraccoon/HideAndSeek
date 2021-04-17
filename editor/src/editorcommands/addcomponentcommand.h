#pragma once

#include "editorcommand.h"

#include "ECS/Entity.h"
#include "ECS/ComponentFactory.h"

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
