#include "addcomponentcommand.h"

#include "src/editorutils/componentreferenceutils.h"

AddComponentCommand::AddComponentCommand(const ComponentSourceReference& source, const StringId typeName, const ComponentFactory& factory)
	: EditorCommand(EffectBitset(EffectType::Components))
	, mSource(source)
	, mComponentTypeName(typeName)
	, mComponentFactory(factory)
{
}

void AddComponentCommand::doCommand(CommandExecutionContext& context)
{
	Utils::AddComponent(
		mSource,
		TypedComponent(mComponentTypeName, mComponentFactory.createComponent(mComponentTypeName)),
		context.world
	);
}

void AddComponentCommand::undoCommand(CommandExecutionContext& context)
{
	Utils::RemoveComponent(
		mSource,
		mComponentTypeName,
		context.world
	);
}
