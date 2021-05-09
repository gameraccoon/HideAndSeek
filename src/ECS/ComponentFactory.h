#pragma once

#include <functional>
#include <optional>
#include <unordered_map>

#include "ErrorHandling.h"

namespace Ecs
{
	template <typename ComponentTypeId>
	class ComponentFactoryImpl
	{
	public:
		using CreationFn = std::function<void*()>;
		using DeletionFn = std::function<void(void*)>;

		ComponentFactoryImpl() = default;
		ComponentFactoryImpl(ComponentFactoryImpl&) = delete;
		ComponentFactoryImpl& operator=(ComponentFactoryImpl&) = delete;
		ComponentFactoryImpl(ComponentFactoryImpl&&) = delete;
		ComponentFactoryImpl& operator=(ComponentFactoryImpl&&) = delete;

		template<typename T>
		void registerComponent()
		{
			mComponentCreators[T::GetTypeName()] = []{
				return new T();
			};
			mComponentDeleters[T::GetTypeName()] = [](void* component){
				return delete static_cast<T*>(component);
			};
		}

		[[nodiscard]] CreationFn getCreationFn(ComponentTypeId className) const
		{
			const auto& it = mComponentCreators.find(className);
			if (it != mComponentCreators.cend())
			{
				return it->second;
			}

#ifdef ECS_DEBUG_CHECKS_ENABLED
			gErrorHandler(std::string("Unknown component type: '") + std::to_string(className) + "'");
#endif // ECS_DEBUG_CHECKS_ENABLED
			return nullptr;
		}

		[[nodiscard]] DeletionFn getDeletionFn(ComponentTypeId className) const
		{
			const auto& it = mComponentDeleters.find(className);
			if (it != mComponentDeleters.cend())
			{
				return it->second;
			}

#ifdef ECS_DEBUG_CHECKS_ENABLED
			gErrorHandler(std::string("Unknown component type: '") + std::to_string(className) + "'");
#endif // ECS_DEBUG_CHECKS_ENABLED
			return nullptr;
		}

		[[nodiscard]] void* createComponent(ComponentTypeId typeName) const
		{
			const auto& it = mComponentCreators.find(typeName);
			if (it != mComponentCreators.cend() && it->second)
			{
				return it->second();
			}

			return nullptr;
		}

		template<typename F>
		void forEachComponentType(F fn) const
		{
			for (auto& creator : mComponentCreators)
			{
				fn(creator.first);
			}
		}

	private:
		std::unordered_map<ComponentTypeId, CreationFn> mComponentCreators;
		std::unordered_map<ComponentTypeId, DeletionFn> mComponentDeleters;
	};

} // namespace Ecs
