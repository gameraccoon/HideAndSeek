#include "addcomponentcommand.h"

#include <QtWidgets/qcombobox.h>

#include <GameData/World.h>

#include "src/editorutils/componentreferenceutils.h"

AddComponentCommand::AddComponentCommand(const ComponentSourceReference& source, StringId typeName, const ComponentFactory& factory)
	: EditorCommand(EffectBitset(EffectType::Components))
	, mSource(source)
	, mComponentTypeName(typeName)
	, mComponentFactory(factory)
{
}

void AddComponentCommand::doCommand(World* world)
{
	Utils::AddComponent(
		mSource,
		mComponentFactory.createComponent(mComponentTypeName),
		world
	);
}

void AddComponentCommand::undoCommand(World* world)
{
	Utils::RemoveComponent(
		mSource,
		mComponentTypeName,
		world
	);
}
