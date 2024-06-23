#pragma once

#include <nlohmann/json_fwd.hpp>

#include "GameData/EcsDefinitions.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class GameData
{
public:
	explicit GameData(const ComponentFactory& componentFactory);

	ComponentSetHolder& getGameComponents() { return mGameComponents; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder) const;
	void fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder);

private:
	ComponentSetHolder mGameComponents;
};
