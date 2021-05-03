#include "Base/precomp.h"

#include "ECS/Entity.h"

#include <nlohmann/json.hpp>

namespace Ecs
{
	void to_json(nlohmann::json& outJson, Entity entity)
	{
		outJson = nlohmann::json{{"id", entity.getId()}};
	}

	void from_json(const nlohmann::json& json, Entity& outEntity)
	{
		outEntity = Entity(json.at("id").get<Entity::EntityId>());
	}

	void to_json(nlohmann::json& outJson, const OptionalEntity& entity)
	{
		outJson = nlohmann::json{
			{"valid", entity.isValid()},
			{"id", entity.getId()}
		};
	}

	void from_json(const nlohmann::json& json, OptionalEntity& outEntity)
	{
		if (json.at("valid").get<bool>())
		{
			outEntity = OptionalEntity(json.at("id").get<Entity::EntityId>());
		}
		else
		{
			outEntity = OptionalEntity();
		}
	}
} // namespace Ecs
