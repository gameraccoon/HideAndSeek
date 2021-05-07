#include "Entity.h"
#include "ErrorHandling.h"

namespace Ecs
{
	Entity OptionalEntity::getEntity() const noexcept {
#ifdef ECS_DEBUG_CHECKS_ENABLED
		if (!mIsValid)
		{
			gErrorHandler("Getting uninitialized entity");
		}
#endif // ECS_DEBUG_CHECKS_ENABLED
		return Entity(mId);
	}

	Entity::EntityId OptionalEntity::getId() const noexcept
	{
#ifdef ECS_DEBUG_CHECKS_ENABLED
		if (!mIsValid)
		{
			gErrorHandler("Getting uninitialized entity");
		}
#endif // ECS_DEBUG_CHECKS_ENABLED
		return mId;
	}

	static_assert(sizeof(Entity) == sizeof(unsigned int), "Entity is too big");
	static_assert(std::is_trivially_copyable<Entity>(), "Entity should be trivially copyable");
	static_assert(std::is_trivially_copyable<OptionalEntity>(), "OptionalEntity should be trivially copyable");

} // namespace Ecs
