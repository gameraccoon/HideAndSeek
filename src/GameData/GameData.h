#pragma once

#include <nlohmann/json_fwd.hpp>

#include "GameData/EcsDefinitions.h"

class GameData
{
public:
	GameData(const ComponentFactory& componentFactory);

	ComponentSetHolder& getGameComponents() { return mGameComponents; }

	[[nodiscard]] nlohmann::json toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers);

private:
	ComponentSetHolder mGameComponents;
};
