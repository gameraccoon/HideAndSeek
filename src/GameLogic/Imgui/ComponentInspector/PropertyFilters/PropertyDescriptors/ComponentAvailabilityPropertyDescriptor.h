#pragma once

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyDescriptor.h"

namespace ImguiPropertyFiltration
{
	class ComponentAvailabilityPropertyDescriptor : public AbstractPropertyDescriptor
	{
	public:
		ComponentAvailabilityPropertyDescriptor(const std::string& componentName, StringId typeId);

		static std::shared_ptr<ComponentAvailabilityPropertyDescriptor> Create(const std::string& componentName, StringId typeId);

		std::any getPropertyValue(EntityManager& /*entityManager*/, Entity /*entity*/) override { return nullptr; }
		[[nodiscard]] StringId getComponentType() const override { return mTypeId; }

	private:
		StringId mTypeId;
	};
}
