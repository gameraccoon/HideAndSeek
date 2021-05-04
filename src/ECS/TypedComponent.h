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

	template <typename ComponentTypeId>
	struct ConstTypedComponentImpl {
		ConstTypedComponentImpl(ComponentTypeId typeId, const void* component)
			: typeId(typeId)
			, component(component)
		{}

		ComponentTypeId typeId;
		const void* component;
	};
} // namespace Ecs
