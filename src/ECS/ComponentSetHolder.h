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
		using ConstTypedComponent = ConstTypedComponentImpl<ComponentTypeId>;
		using ComponentFactory = ComponentFactoryImpl<ComponentTypeId>;

		ComponentSetHolderImpl(const ComponentFactory& componentFactory)
			: mComponentFactory(componentFactory)
		{}

		~ComponentSetHolderImpl()
		{
			removeAllComponents();
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

		std::vector<ConstTypedComponent> getAllComponents() const
		{
			std::vector<ConstTypedComponent> components;

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

		void* addComponentByType(ComponentTypeId typeId) noexcept
		{
			auto createFn = mComponentFactory.getCreationFn(typeId);
			void* component = createFn();
			addComponent(component, typeId);
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
			if (component != nullptr && !mComponents.contains(typeId))
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

		[[nodiscard]] bool hasAnyComponents() const
		{
			return !mComponents.empty();
		}

		void removeAllComponents()
		{
			for (auto& component : mComponents)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(component.first);
				deleterFn(component.second);
			}
			mComponents.clear();
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
