#pragma once

#include "Base/Types/TemplateAliases.h"

#include "GameData/Spatial/WorldCell.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyDescriptor.h"

namespace ImguiPropertyFiltration
{
	class AbstractPropertyFilter
	{
	public:
		AbstractPropertyFilter(const std::weak_ptr<AbstractPropertyDescriptor>& descriptor)
			: mDescriptor(descriptor)
		{}

		virtual ~AbstractPropertyFilter() = default;

		std::type_index getComponentType() const
		{
			return getDescriptor()->getComponentType();
		}

		void filterEntities(TupleVector<WorldCell*, Entity>& inOutEntities)
		{
			inOutEntities.erase(
				std::remove_if(
					inOutEntities.begin(),
					inOutEntities.end(),
					[this](const auto& tuple)
					{
						return !isConditionPassed(std::get<0>(tuple)->getEntityManager(), std::get<1>(tuple));
					}
				),
				inOutEntities.end()
			);
		}

		virtual std::string getName() const = 0;
		virtual void updateImguiWidget() = 0;

	protected:
		virtual bool isConditionPassed(EntityManager& manager, Entity entity) const = 0;

		const std::shared_ptr<AbstractPropertyDescriptor> getDescriptor() const
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
		virtual std::string getName() const = 0;
		virtual std::unique_ptr<AbstractPropertyFilter> createFilter() const = 0;
	};

	template<typename T>
	class PropertyFilterFactory : public AbstractPropertyFilterFactory
	{
	public:
		PropertyFilterFactory(const std::weak_ptr<AbstractPropertyDescriptor>& descriptor)
			: mDescriptor(descriptor)
		{}

		std::string getName() const final { return T::GetStaticName(); }

		std::unique_ptr<AbstractPropertyFilter> createFilter() const final
		{
			return std::make_unique<T>(mDescriptor);
		}

	private:
		std::weak_ptr<AbstractPropertyDescriptor> mDescriptor;
	};

} // namespace ImguiPropertyFiltration
