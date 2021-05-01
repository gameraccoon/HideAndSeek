#pragma once

#include "ECS/ComponentFactory.h"
#include "ECS/Serialization/JsonComponentSerializer.h"

namespace Ecs
{
	// Aggregates everything needed to construct, serialize or deserialize a component
	struct ComponentSerializersHolder
	{
		JsonComponentSerializationHolder jsonSerializer;

		ComponentSerializersHolder() = default;
		ComponentSerializersHolder(ComponentSerializersHolder&) = delete;
		ComponentSerializersHolder& operator=(ComponentSerializersHolder&) = delete;
	};

} // namespace Ecs
