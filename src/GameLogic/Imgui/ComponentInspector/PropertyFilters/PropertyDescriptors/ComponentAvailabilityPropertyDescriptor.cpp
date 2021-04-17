#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/PropertyDescriptors/ComponentAvailabilityPropertyDescriptor.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/HasComponentPropertyFilter.h"

namespace ImguiPropertyFiltration
{
	ComponentAvailabilityPropertyDescriptor::ComponentAvailabilityPropertyDescriptor(const std::string& componentName, StringId typeId)
		: AbstractPropertyDescriptor(componentName)
		, mTypeId(typeId)
	{}

	std::shared_ptr<ComponentAvailabilityPropertyDescriptor> ComponentAvailabilityPropertyDescriptor::Create(const std::string& componentName, StringId typeId)
	{
		auto result = std::make_shared<ComponentAvailabilityPropertyDescriptor>(componentName, typeId);
		result->mFilterFactories.push_back(std::make_shared<PropertyFilterFactory<HasComponentPropertyFilter>>(result));
		return result;
	}
} // namespace ImguiPropertyFiltration
