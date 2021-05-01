#include "removecomponentcommand.h"

#include <QtWidgets/qcombobox.h>

#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "GameData/World.h"

#include "src/editorutils/componentreferenceutils.h"

RemoveComponentCommand::RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const Ecs::ComponentSerializersHolder& serializerHolder, const ComponentFactory& componentFactory)
	: EditorCommand(EffectBitset(EffectType::Components))
	, mSource(source)
	, mComponentTypeName(typeName)
	, mComponentSerializerHolder(serializerHolder)
	, mComponentFactory(componentFactory)
{
}

void RemoveComponentCommand::doCommand(World* world)
{
	if (mSerializedComponent.empty())
	{
		std::vector<TypedComponent> components = Utils::GetComponents(mSource, world);

		auto it = std::find_if(components.begin(), components.end(), [typeName = mComponentTypeName](const TypedComponent& component)
		{
			return component.typeId == typeName;
		});

		if (it == components.end())
		{
			return;
		}

		const Ecs::JsonComponentSerializer* jsonSerializer = mComponentSerializerHolder.jsonSerializer.getComponentSerializerFromClassName(mComponentTypeName);
		jsonSerializer->toJson(mSerializedComponent, it->component);
	}

	Utils::RemoveComponent(
		mSource,
		mComponentTypeName,
		world
	);
}

void RemoveComponentCommand::undoCommand(World* world)
{
	void* component = mComponentFactory.createComponent(mComponentTypeName);

	const Ecs::JsonComponentSerializer* jsonSerializer = mComponentSerializerHolder.jsonSerializer.getComponentSerializerFromClassName(mComponentTypeName);
	jsonSerializer->fromJson(mSerializedComponent, component);

	Utils::AddComponent(
		mSource,
		TypedComponent(mComponentTypeName, component),
		world
	);
}
