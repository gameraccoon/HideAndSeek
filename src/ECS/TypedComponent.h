#pragma once

namespace Ecs
{
	template <typename ComponentTypeId>
	struct TypedComponentImpl {
		TypedComponentImpl(ComponentTypeId typeId, void* component)
			: typeId(typeId)
			, component(component)
		{}

		ComponentTypeId typeId;
		void* component;
	};
} // namespace Ecs
