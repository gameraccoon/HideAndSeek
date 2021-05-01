#pragma once

#include <nlohmann/json_fwd.hpp>

namespace Ecs
{
	class Entity
	{
	public:
		using EntityID = unsigned int;

	public:
		explicit Entity(EntityID id) : mId(id) {}

		bool operator ==(Entity b) const { return mId == b.mId; }
		bool operator !=(Entity b) const { return !(*this == b); }
		bool operator <(Entity b) const { return mId < b.mId; }

		[[nodiscard]] EntityID getID() const { return mId; }

		friend void to_json(nlohmann::json& outJson, Entity entity);
		friend void from_json(const nlohmann::json& json, Entity& outEntity);

	private:
		EntityID mId;
	};

	class OptionalEntity
	{
	public:
		OptionalEntity() = default;
		// implicit conversion
		OptionalEntity(Entity entity) : mId(entity.getID()), mIsValid(true) {}
		explicit OptionalEntity(Entity::EntityID id) : mId(id), mIsValid(true) {}

		[[nodiscard]] bool isValid() const { return mIsValid; }
		[[nodiscard]] Entity getEntity() const noexcept;
		[[nodiscard]] Entity::EntityID getID() const noexcept;

		friend void to_json(nlohmann::json& outJson, const OptionalEntity& entity);
		friend void from_json(const nlohmann::json& json, OptionalEntity& outEntity);

	private:
		Entity::EntityID mId = 0;
		bool mIsValid = false;
	};
} // namespace Ecs
