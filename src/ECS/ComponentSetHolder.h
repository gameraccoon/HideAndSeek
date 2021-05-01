#pragma once

#include <tuple>
#include <map>
#include <vector>

#include <nlohmann/json.hpp>

#include "ECS/Delegates.h"
#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "ECS/TypedComponent.h"

namespace Ecs
{
	/**
	 * Use this class to store components specific for some non-entity object (e.g. for a World)
	 */
	template <typename ComponentTypeId>
	class ComponentSetHolderImpl
	{
	public:
		using TypedComponent = TypedComponentImpl<ComponentTypeId>;
		using ComponentFactory = ComponentFactoryImpl<ComponentTypeId>;

		ComponentSetHolderImpl(const ComponentFactory& componentFactory)
			: mComponentFactory(componentFactory)
		{}

		~ComponentSetHolderImpl()
		{
			for (auto& component : mComponents)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(component.first);
				deleterFn(component.second);
			}
		}

		ComponentSetHolderImpl(const ComponentSetHolderImpl&) = delete;
		ComponentSetHolderImpl& operator=(const ComponentSetHolderImpl&) = delete;
		ComponentSetHolderImpl(ComponentSetHolderImpl&&) = default;
		ComponentSetHolderImpl& operator=(ComponentSetHolderImpl&&) = default;

		std::vector<TypedComponent> getAllComponents()
		{
			std::vector<TypedComponent> components;

			for (auto& componentData : mComponents)
			{
				components.emplace_back(componentData.first, componentData.second);
			}

			return components;
		}

		template<typename ComponentType>
		bool doesComponentExists()
		{
			return mComponents[ComponentType::GetTypeName()] != nullptr;
		}

		template<typename T>
		T* addComponent() noexcept
		{
			T* component = HS_NEW T();
			addComponent(component, T::GetTypeName());
			return component;
		}

		template<typename T>
		T* getOrAddComponent()
		{
			auto it = mComponents.find(T::GetTypeName());
			if (it == mComponents.end())
			{
				it = mComponents.emplace(T::GetTypeName(), HS_NEW T()).first;
			}
			return static_cast<T*>(it->second);
		}

		void addComponent(void* component, ComponentTypeId typeId)
		{
			if (component != nullptr)
			{
				mComponents[typeId] = component;
			}
		}

		void removeComponent(ComponentTypeId typeId)
		{
			if (auto it = mComponents.find(typeId); it != mComponents.end())
			{
				auto deleterFn = mComponentFactory.getDeletionFn(it->first);
				deleterFn(it->second);
				mComponents.erase(it);
			}
		}

		template<typename... Components>
		std::tuple<Components*...> getComponents()
		{
			return std::make_tuple(getSingleComponent<Components>()...);
		}

		template<typename... Components>
		std::tuple<const Components*...> getComponents() const
		{
			return std::make_tuple(getSingleComponent<Components>()...);
		}

		[[nodiscard]] nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const
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

		void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
		{
			const auto& components = json.at("components");
			for (const auto& [stringType, componentData] : components.items())
			{
				StringId className = STR_TO_ID(stringType);
				typename ComponentFactory::CreationFn componentCreateFn = mComponentFactory.getCreationFn(className);
				if (componentCreateFn != nullptr)
				{
					if (!componentData.is_null())
					{
						void* component = componentCreateFn();
						componentSerializers.jsonSerializer.getComponentSerializerFromClassName(className)->fromJson(componentData, component);
						mComponents[className] = component;
					}
				}
			}
		}

		[[nodiscard]] bool hasAnyComponents() const
		{
			return !mComponents.empty();
		}

	private:
		template<typename Component>
		Component* getSingleComponent()
		{
			auto it = mComponents.find(Component::GetTypeName());
			if (it != mComponents.end())
			{
				return static_cast<Component*>(it->second);
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Component>
		const Component* getSingleComponent() const
		{
			auto it = mComponents.find(Component::GetTypeName());
			if (it != mComponents.end())
			{
				return static_cast<Component*>(it->second);
			}
			else
			{
				return nullptr;
			}
		}

	private:
		std::unordered_map<ComponentTypeId, void*> mComponents;

		// it's a temporary solution to store it here
		const ComponentFactory& mComponentFactory;
	};

} // namespace Ecs
