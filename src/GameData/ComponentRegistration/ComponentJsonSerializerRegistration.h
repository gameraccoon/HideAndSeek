#pragma once

#include "GameData/Serialization/Json/JsonComponentSerializer.h"

namespace ComponentsRegistration
{
	void RegisterJsonSerializers(Json::ComponentSerializationHolder& jsonSerializerHolder);
}
