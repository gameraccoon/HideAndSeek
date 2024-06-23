#pragma once

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyFilter.h"

namespace ImguiPropertyFiltration
{
	class FloatGreaterPropertyFilter final : public AbstractPropertyFilter
	{
	public:
		using AbstractPropertyFilter::AbstractPropertyFilter;

		static std::string GetStaticName();

		[[nodiscard]] std::string getName() const override;
		void updateImguiWidget() override;
		bool isConditionPassed(EntityManager& manager, Entity entity) const override;

	private:
		float mMinValue = 0.0f;
	};

	class FloatLessPropertyFilter final : public AbstractPropertyFilter
	{
	public:
		using AbstractPropertyFilter::AbstractPropertyFilter;

		static std::string GetStaticName();

		[[nodiscard]] std::string getName() const override;
		void updateImguiWidget() override;
		bool isConditionPassed(EntityManager& manager, Entity entity) const override;

	private:
		float mMaxValue = 0.0f;
	};
}
