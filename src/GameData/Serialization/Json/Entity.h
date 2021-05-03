#pragma once

#include "ECS/Entity.h"
#include <nlohmann/json_fwd.hpp>

namespace Ecs
{
	void to_json(nlohmann::json& outJson, Entity entity);
	void from_json(const nlohmann::json& json, Entity& outEntity);

	void to_json(nlohmann::json& outJson, const OptionalEntity& entity);
	void from_json(const nlohmann::json& json, OptionalEntity& outEntity);
} // namespace Ecs
