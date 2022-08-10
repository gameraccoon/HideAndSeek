#include "Base/precomp.h"

#ifdef IMGUI_ENABLED

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/FloatPropertyFilters.h"

#include "imgui.h"

namespace ImguiPropertyFiltration
{
	std::string FloatGreaterPropertyFilter::GetStaticName()
	{
		return "greater than";
	}

	std::string FloatGreaterPropertyFilter::getName() const
	{
		return getDescriptor()->getName() + FormatString(" is greater than %f", mMinValue);
	}

	void FloatGreaterPropertyFilter::updateImguiWidget()
	{
		std::string name = getDescriptor()->getName() + " is greater than ";
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::InputFloat(std::string("##").append(name).c_str(), &mMinValue);
	}

	bool FloatGreaterPropertyFilter::isConditionPassed(EntityManager& manager, Entity entity) const
	{
		float val = std::any_cast<float>(getDescriptor()->getPropertyValue(manager, entity));
		return val > mMinValue;
	}

	std::string FloatLessPropertyFilter::GetStaticName()
	{
		return "less than";
	}

	std::string FloatLessPropertyFilter::getName() const
	{
		return getDescriptor()->getName() + FormatString(" is less than %f", mMaxValue);
	}

	void FloatLessPropertyFilter::updateImguiWidget()
	{
		std::string name = getDescriptor()->getName() + " is less than ";
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::InputFloat(std::string("##").append(name).c_str(), &mMaxValue);
	}

	bool FloatLessPropertyFilter::isConditionPassed(EntityManager& manager, Entity entity) const
	{
		float val = std::any_cast<float>(getDescriptor()->getPropertyValue(manager, entity));
		return val < mMaxValue;
	}
}

#endif // IMGUI_ENABLED
