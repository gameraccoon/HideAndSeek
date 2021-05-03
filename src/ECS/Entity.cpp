#include "Base/precomp.h"

#include "ECS/Entity.h"

namespace Ecs
{
	Entity OptionalEntity::getEntity() const noexcept {
		Assert(mIsValid, "Getting uninitialized entity");
		return Entity(mId);
	}

	Entity::EntityId OptionalEntity::getId() const noexcept
	{
		Assert(mIsValid, "Getting uninitialized entity ID");
		return mId;
	}

	static_assert(sizeof(Entity) == sizeof(unsigned int), "Entity is too big");
	static_assert(std::is_trivially_copyable<Entity>(), "Entity should be trivially copyable");
	static_assert(std::is_trivially_copyable<OptionalEntity>(), "OptionalEntity should be trivially copyable");

} // namespace Ecs
