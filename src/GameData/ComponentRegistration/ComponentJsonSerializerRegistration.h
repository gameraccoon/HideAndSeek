#pragma once

#include "ECS/Serialization/JsonComponentSerializer.h"

namespace ComponentsRegistration
{
	void RegisterJsonSerializers(Ecs::JsonComponentSerializationHolder& jsonSerializerHolder);
}
