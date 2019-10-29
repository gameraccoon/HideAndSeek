#include "Entity.h"

#include <nlohmann/json.hpp>

#include "Base/Debug/Assert.h"

void to_json(nlohmann::json& outJson, Entity entity)
{
	outJson = nlohmann::json{{"id", entity.mId}};
}

void from_json(const nlohmann::json& json, Entity& outEntity)
{
	json.at("id").get_to(outEntity.mId);
}

Entity OptionalEntity::getEntity() const {
	Assert(mIsValid, "Getting uninitialized entity");
	return Entity(mId);
}

void to_json(nlohmann::json& outJson, const OptionalEntity& entity)
{
	outJson = nlohmann::json{
		{"valid", entity.mIsValid},
		{"id", entity.mId}
	};
}

void from_json(const nlohmann::json& json, OptionalEntity& outEntity)
{
	json.at("valid").get_to(outEntity.mIsValid);
	json.at("id").get_to(outEntity.mId);
}

static_assert(sizeof(Entity) == sizeof(unsigned int), "Entity is too big");
static_assert(std::is_trivially_copyable<Entity>(), "Entity should be trivially copyable");
static_assert(std::is_trivially_copyable<OptionalEntity>(), "OptionalEntity should be trivially copyable");
