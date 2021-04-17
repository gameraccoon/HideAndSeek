#pragma once

#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

/**
 * @brief Non-owning wrapper around entity amd its current entity manager
 */
class EntityView
{
public:
	EntityView(Entity entity, EntityManager& manager);

	template<typename ComponentType>
	ComponentType* addComponent()
	{
		return mManager.addComponent<ComponentType>(mEntity);
	}

	template<typename ComponentType>
	void removeComponent()
	{
		mManager.removeComponent<ComponentType>(mEntity);
	}

	template<typename... Components>
	std::tuple<Components*...> getComponents()
	{
		return mManager.getEntityComponents<Components...>(mEntity);
	}

	template<typename ComponentType>
	ComponentType* scheduleAddComponent()
	{
		return mManager.scheduleAddComponent<ComponentType>(mEntity);
	}

	template<typename ComponentType>
	void scheduleRemoveComponent()
	{
		mManager.scheduleRemoveComponent<ComponentType>(mEntity);
	}

	[[nodiscard]] Entity getEntity() const;
	EntityManager& getManager() { return mManager; }

private:
	Entity mEntity;
	EntityManager& mManager;
};
