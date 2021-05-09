#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "ErrorHandling.h"
#include "ComponentPool.h"

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

		template<typename ComponentType, std::size_t PageSize = std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(4096u / sizeof(ComponentType)))>
		void registerComponent(size_t preallocatedPagesCount = 0)
		{
			const ComponentTypeId componentTypeId = ComponentType::GetTypeName();

			auto componentPoolRawPtr = new (std::nothrow) ComponentPool<ComponentType, PageSize>(preallocatedPagesCount);
			mComponentPools.emplace_back(componentPoolRawPtr);

			mComponentCreators[componentTypeId] = [componentPoolRawPtr]{
				return componentPoolRawPtr->acquireComponent();
			};
			mComponentDeleters[componentTypeId] = [componentPoolRawPtr](void* component){
				if (component) {
					return componentPoolRawPtr->releaseComponent(component);
				}
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
		std::vector<std::unique_ptr<ComponentPoolBase>> mComponentPools;
	};

} // namespace Ecs
