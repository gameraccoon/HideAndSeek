#pragma once

#include "ECS/Component.h"


class JsonComponentSerializer
{
public:
	virtual ~JsonComponentSerializer() = default;
	virtual void toJson(nlohmann::json& outJson, const BaseComponent* component) const = 0;
	virtual void fromJson(const nlohmann::json& json, BaseComponent* outComponent) const = 0;
};

class JsonComponentSerializationHolder
{
public:
	JsonComponentSerializationHolder() = default;
	JsonComponentSerializationHolder(JsonComponentSerializationHolder&) = delete;
	JsonComponentSerializationHolder& operator=(JsonComponentSerializationHolder&) = delete;

	const JsonComponentSerializer* getComponentSerializerFromClassName(StringID className) const
	{
		const auto& it = mClassNameToSerializer.find(className);
		if (it != mClassNameToSerializer.end())
		{
			return mSerializers[it->second].get();
		}

		ReportFatalError("Unknown component type: '%s'", className);
		return nullptr;
	}

	template<typename T>
	void registerSerializer(StringID className, std::unique_ptr<JsonComponentSerializer>&& serializer)
	{
		mSerializers.push_back(std::move(serializer));
		mClassNameToSerializer.emplace(className, mSerializers.size() - 1);
	}

private:
	std::vector<std::unique_ptr<JsonComponentSerializer>> mSerializers;
	std::unordered_map<StringID, size_t> mClassNameToSerializer;
};
