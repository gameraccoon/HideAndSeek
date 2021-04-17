#include "Base/precomp.h"

#include "ECS/ComponentSetHolder.h"

#include <nlohmann/json.hpp>

#include "ECS/Serialization/ComponentSerializersHolder.h"

ComponentSetHolder::~ComponentSetHolder()
{
	for (auto& component : mComponents)
	{
		delete component.second;
	}
}

std::vector<BaseComponent*> ComponentSetHolder::getAllComponents()
{
	std::vector<BaseComponent*> components;

	for (auto& componentArray : mComponents)
	{
		components.push_back(componentArray.second);
	}

	return components;
}

void ComponentSetHolder::addComponent(BaseComponent* component, StringId typeId)
{
	if (component != nullptr)
	{
		mComponents[typeId] = component;
	}
}

void ComponentSetHolder::removeComponent(StringId typeId)
{
	if (auto it = mComponents.find(typeId); it != mComponents.end())
	{
		delete it->second;
		mComponents.erase(it);
	}
}

nlohmann::json ComponentSetHolder::toJson(const ComponentSerializersHolder& componentSerializers) const
{
	nlohmann::json outJson;

	auto components = nlohmann::json{};

	for (auto component : mComponents)
	{
		auto componentObj = nlohmann::json{};
		componentSerializers.jsonSerializer.getComponentSerializerFromClassName(component.first)->toJson(componentObj, component.second);
		components[ID_TO_STR(component.first)] = componentObj;
	}
	outJson["components"] = components;

	return outJson;
}

void ComponentSetHolder::fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
{
	const auto& components = json.at("components");
	for (const auto& [stringType, componentData] : components.items())
	{
		StringId className = STR_TO_ID(stringType);
		ComponentFactory::CreationFn componentCreateFn = componentSerializers.factory.getCreationFn(className);
		if (componentCreateFn != nullptr)
		{
			if (!componentData.is_null())
			{
				BaseComponent* component = componentCreateFn();
				componentSerializers.jsonSerializer.getComponentSerializerFromClassName(className)->fromJson(componentData, component);
				mComponents[className] = component;
			}
		}
	}
}

bool ComponentSetHolder::hasAnyComponents() const
{
	return !mComponents.empty();
}
