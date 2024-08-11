#pragma once

#include <algorithm>

#include "EngineCommon/Debug/Assert.h"
#include "EngineCommon/Types/TemplateAliases.h"

#include "GameData/Spatial/WorldCell.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyDescriptor.h"

namespace ImguiPropertyFiltration
{
	class AbstractPropertyFilter
	{
	public:
		explicit AbstractPropertyFilter(const std::weak_ptr<AbstractPropertyDescriptor>& descriptor)
			: mDescriptor(descriptor)
		{}

		virtual ~AbstractPropertyFilter() = default;

		[[nodiscard]] StringId getComponentType() const
		{
			return getDescriptor()->getComponentType();
		}

		void filterEntities(TupleVector<WorldCell*, Entity>& inOutEntities)
		{
			std::erase_if(
				inOutEntities,
				[this](const auto& tuple) {
					return !isConditionPassed(
						std::get<0>(tuple)->getEntityManager(),
						std::get<1>(tuple)
					);
				}
			);
		}

		[[nodiscard]] virtual std::string getName() const = 0;
		virtual void updateImguiWidget() = 0;

	protected:
		virtual bool isConditionPassed(EntityManager& manager, Entity entity) const = 0;

		[[nodiscard]] std::shared_ptr<AbstractPropertyDescriptor> getDescriptor() const
		{
			std::shared_ptr<AbstractPropertyDescriptor> descriptor = mDescriptor.lock();
			AssertFatal(descriptor, "Property descriptor should exist for PropertyFilter");
			return descriptor;
		}

	private:
		std::weak_ptr<AbstractPropertyDescriptor> mDescriptor;
	};

	class AbstractPropertyFilterFactory
	{
	public:
		virtual ~AbstractPropertyFilterFactory() = default;
		[[nodiscard]] virtual std::string getName() const = 0;
		[[nodiscard]] virtual std::unique_ptr<AbstractPropertyFilter> createFilter() const = 0;
	};

	template<typename T>
	class PropertyFilterFactory final : public AbstractPropertyFilterFactory
	{
	public:
		explicit PropertyFilterFactory(const std::weak_ptr<AbstractPropertyDescriptor>& descriptor)
			: mDescriptor(descriptor)
		{}

		[[nodiscard]] std::string getName() const override { return T::GetStaticName(); }

		[[nodiscard]] std::unique_ptr<AbstractPropertyFilter> createFilter() const override
		{
			return std::make_unique<T>(mDescriptor);
		}

	private:
		std::weak_ptr<AbstractPropertyDescriptor> mDescriptor;
	};

} // namespace ImguiPropertyFiltration
