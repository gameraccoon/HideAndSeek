#pragma once

namespace Ecs
{
	class JsonComponentSerializer
	{
	public:
		virtual ~JsonComponentSerializer() = default;
		virtual void toJson(nlohmann::json& outJson, const void* component) const = 0;
		virtual void fromJson(const nlohmann::json& json, void* outComponent) const = 0;
	};

	class JsonComponentSerializationHolder
	{
	public:
		JsonComponentSerializationHolder() = default;
		JsonComponentSerializationHolder(JsonComponentSerializationHolder&) = delete;
		JsonComponentSerializationHolder& operator=(JsonComponentSerializationHolder&) = delete;

		[[nodiscard]] const JsonComponentSerializer* getComponentSerializerFromClassName(StringId className) const
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
		void registerSerializer(StringId className, std::unique_ptr<JsonComponentSerializer>&& serializer)
		{
			mSerializers.push_back(std::move(serializer));
			mClassNameToSerializer.emplace(className, mSerializers.size() - 1);
		}

	private:
		std::vector<std::unique_ptr<JsonComponentSerializer>> mSerializers;
		std::unordered_map<StringId, size_t> mClassNameToSerializer;
	};

} // namespace Ecs
