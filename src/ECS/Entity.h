#pragma once

namespace Ecs
{
	class Entity
	{
	public:
		using EntityId = unsigned int;

	public:
		explicit Entity(EntityId id) : mId(id) {}

		bool operator ==(Entity b) const { return mId == b.mId; }
		bool operator !=(Entity b) const { return !(*this == b); }
		bool operator <(Entity b) const { return mId < b.mId; }

		[[nodiscard]] EntityId getId() const { return mId; }

	private:
		EntityId mId;
	};

	class OptionalEntity
	{
	public:
		OptionalEntity() = default;
		// implicit conversion
		OptionalEntity(Entity entity) : mId(entity.getId()), mIsValid(true) {}
		explicit OptionalEntity(Entity::EntityId id) : mId(id), mIsValid(true) {}

		[[nodiscard]] bool isValid() const { return mIsValid; }
		[[nodiscard]] Entity getEntity() const noexcept;
		[[nodiscard]] Entity::EntityId getId() const noexcept;

	private:
		Entity::EntityId mId = 0;
		bool mIsValid = false;
	};
} // namespace Ecs
