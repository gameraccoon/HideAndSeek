#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/Vector2DPropertyFIlters.h"

#include "imgui.h"

namespace ImguiPropertyFiltration
{
	std::string Vector2DInRectPropertyFilter::GetStaticName()
	{
		return "in rect";
	}

	std::string Vector2DInRectPropertyFilter::getName() const
	{
		return getDescriptor()->getName() + FormatString(" is in rect (%f, %f) (%f, %f)", mLT.x, mLT.y, mRB.x, mRB.y);
	}

	void Vector2DInRectPropertyFilter::updateImguiWidget()
	{
		std::string nameStart = getDescriptor()->getName() + " is in rect (";
		ImGui::TextUnformatted(nameStart.c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat(std::string("##").append(nameStart).append("ltx").c_str(), &mLT.x);
		ImGui::SameLine();
		ImGui::TextUnformatted(",");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat(std::string("##").append(nameStart).append("lty").c_str(), &mLT.y);
		ImGui::SameLine();
		ImGui::TextUnformatted("),(");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat(std::string("##").append(nameStart).append("rbx").c_str(), &mRB.x);
		ImGui::SameLine();
		ImGui::TextUnformatted(",");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat(std::string("##").append(nameStart).append("rby").c_str(), &mRB.y);
		ImGui::SameLine();
		ImGui::TextUnformatted(")");
	}

	bool Vector2DInRectPropertyFilter::isConditionPassed(EntityManager& manager, Entity entity) const
	{
		Vector2D val = std::any_cast<Vector2D>(getDescriptor()->getPropertyValue(manager, entity));
		return mLT.x < val.x && val.x < mRB.x && mLT.y < val.y && val.y < mRB.y;
	}
}
