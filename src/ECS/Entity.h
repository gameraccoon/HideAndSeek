#pragma once

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

	private:
		Entity::EntityID mId = 0;
		bool mIsValid = false;
	};
} // namespace Ecs
