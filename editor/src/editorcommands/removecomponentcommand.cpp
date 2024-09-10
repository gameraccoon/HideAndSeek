#include "removecomponentcommand.h"

#include "src/editorutils/componentreferenceutils.h"

#include "GameData/World.h"

RemoveComponentCommand::RemoveComponentCommand(const ComponentSourceReference& source, StringId typeName, const Json::ComponentSerializationHolder& jsonSerializerHolder, const ComponentFactory& componentFactory)
	: EditorCommand(EffectBitset(EffectType::Components))
	, mSource(source)
	, mComponentTypeName(typeName)
	, mComponentSerializerHolder(jsonSerializerHolder)
	, mComponentFactory(componentFactory)
{
}

void RemoveComponentCommand::doCommand(CommandExecutionContext& context)
{
	if (mSerializedComponent.empty())
	{
		std::vector<TypedComponent> components = Utils::GetComponents(mSource, context.world);

		auto it = std::find_if(components.begin(), components.end(), [typeName = mComponentTypeName](const TypedComponent& component) {
			return component.typeId == typeName;
		});

		if (it == components.end())
		{
			return;
		}

		const Json::ComponentSerializer* jsonSerializer = mComponentSerializerHolder.getComponentSerializerFromClassName(mComponentTypeName);
		jsonSerializer->toJson(mSerializedComponent, it->component);
	}

	Utils::RemoveComponent(
		mSource,
		mComponentTypeName,
		context.world
	);
}

void RemoveComponentCommand::undoCommand(CommandExecutionContext& context)
{
	void* component = mComponentFactory.createComponent(mComponentTypeName);

	const Json::ComponentSerializer* jsonSerializer = mComponentSerializerHolder.getComponentSerializerFromClassName(mComponentTypeName);
	jsonSerializer->fromJson(mSerializedComponent, component);

	Utils::AddComponent(
		mSource,
		TypedComponent(mComponentTypeName, component),
		context.world
	);
}
