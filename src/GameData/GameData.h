#pragma once

#include <nlohmann/json_fwd.hpp>

#include "ECS/ComponentSetHolder.h"

struct ComponentSerializersHolder;

class GameData
{
public:
	ComponentSetHolder& getGameComponents() { return mGameComponents; }

	[[nodiscard]] nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers);

private:
	ComponentSetHolder mGameComponents;
};
