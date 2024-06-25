#include "EngineCommon/precomp.h"

#ifdef IMGUI_ENABLED

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/HasComponentPropertyFilter.h"

#include "imgui.h"

namespace ImguiPropertyFiltration
{
	std::string HasComponentPropertyFilter::GetStaticName()
	{
		return "has component";
	}

	std::string HasComponentPropertyFilter::getName() const
	{
		return std::string("has ").append(getDescriptor()->getName());
	}

	void HasComponentPropertyFilter::updateImguiWidget()
	{
		ImGui::TextUnformatted(getName().c_str());
	}

	bool HasComponentPropertyFilter::isConditionPassed(EntityManager&, Entity) const
	{
		return true;
	}
} // namespace ImguiPropertyFiltration

#endif // IMGUI_ENABLED
