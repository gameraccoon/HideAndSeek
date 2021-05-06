#include "Base/precomp.h"

#include "GameData/Serialization/Json/ComponentSetHolder.h"

#include <nlohmann/json.hpp>

namespace Json
{
	nlohmann::json SerializeComponentSetHolder(const ComponentSetHolder& componentSetHolder, const Json::ComponentSerializationHolder& jsonSerializerHolder)
	{
		nlohmann::json outJson;
		auto components = nlohmann::json{};

		for (const ConstTypedComponent& componentData : componentSetHolder.getAllComponents())
		{
			auto componentObj = nlohmann::json{};
			jsonSerializerHolder.getComponentSerializerFromClassName(componentData.typeId)->toJson(componentObj, componentData.component);
			components[ID_TO_STR(componentData.typeId)] = componentObj;
		}
		outJson["components"] = components;
		return outJson;
	}

	void DeserializeComponentSetHolder(ComponentSetHolder& outComponentSetHolder, const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder)
	{
		outComponentSetHolder.removeAllComponents();
		const auto& components = json.at("components");
		for (const auto& [stringType, componentData] : components.items())
		{
			StringId className = STR_TO_ID(stringType);
			if (!componentData.is_null())
			{
				void* component = outComponentSetHolder.addComponentByType(className);
				jsonSerializerHolder.getComponentSerializerFromClassName(className)->fromJson(componentData, component);
			}
		}
	}
}
