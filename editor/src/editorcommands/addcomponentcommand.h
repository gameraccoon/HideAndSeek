#pragma once

#include "editorcommand.h"
#include "src/editorutils/componentreference.h"
#include <raccoon-ecs/component_factory.h>

class World;

class AddComponentCommand final : public EditorCommand
{
public:
	AddComponentCommand(const ComponentSourceReference& source, StringId typeName, const ComponentFactory& factory);

	void doCommand(CommandExecutionContext& context) override;
	void undoCommand(CommandExecutionContext& context) override;

private:
	ComponentSourceReference mSource;
	StringId mComponentTypeName;
	const ComponentFactory& mComponentFactory;
};
