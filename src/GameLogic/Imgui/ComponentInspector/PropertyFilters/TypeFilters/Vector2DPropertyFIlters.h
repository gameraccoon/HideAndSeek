#pragma once

#include "GameData/Geometry/Vector2D.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyFilter.h"

namespace ImguiPropertyFiltration
{
	class Vector2DInRectPropertyFilter final : public AbstractPropertyFilter
	{
	public:
		using AbstractPropertyFilter::AbstractPropertyFilter;

		static std::string GetStaticName();

		[[nodiscard]] std::string getName() const override;
		void updateImguiWidget() override;
		bool isConditionPassed(EntityManager& manager, Entity entity) const override;

	private:
		Vector2D mLT{0.0f, 0.0f};
		Vector2D mRB{0.0f, 0.0f};
	};
}
