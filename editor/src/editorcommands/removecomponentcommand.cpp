#include "removecomponentcommand.h"

#include <QtWidgets/qcombobox.h>

#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "GameData/World.h"

#include "src/editorutils/componentreferenceutils.h"

RemoveComponentCommand::RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const ComponentSerializersHolder& serializerHolder)
	: EditorCommand(EffectBitset(EffectType::Components))
	, mSource(source)
	, mComponentTypeName(typeName)
	, mComponentSerializerHolder(serializerHolder)
{
}

void RemoveComponentCommand::doCommand(World* world)
{
	if (mSerializedComponent.empty())
	{
		std::vector<BaseComponent*> components = Utils::GetComponents(mSource, world);

		auto it = std::find_if(components.begin(), components.end(), [typeName = mComponentTypeName](BaseComponent* component)
		{
			return component->getComponentTypeName() == typeName;
		});

		if (it == components.end())
		{
			return;
		}

		const JsonComponentSerializer* jsonSerializer = mComponentSerializerHolder.jsonSerializer.getComponentSerializerFromClassName(mComponentTypeName);
		jsonSerializer->toJson(mSerializedComponent, *it);
	}

	Utils::RemoveComponent(
		mSource,
		mComponentTypeName,
		world
	);
}

void RemoveComponentCommand::undoCommand(World* world)
{
	BaseComponent* component = mComponentSerializerHolder.factory.createComponent(mComponentTypeName);

	const JsonComponentSerializer* jsonSerializer = mComponentSerializerHolder.jsonSerializer.getComponentSerializerFromClassName(mComponentTypeName);
	jsonSerializer->fromJson(mSerializedComponent, component);

	Utils::AddComponent(
		mSource,
		component,
		world
	);
}
