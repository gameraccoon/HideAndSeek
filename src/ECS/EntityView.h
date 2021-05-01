#pragma once

#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

namespace Ecs
{
	/**
	 * @brief Non-owning wrapper around entity amd its current entity manager
	 */
	template <typename ComponentTypeId>
	class EntityViewImpl
	{
	public:
		using EntityManager = EntityManagerImpl<ComponentTypeId>;

		EntityViewImpl(Entity entity, EntityManager& manager)
			: mEntity(entity)
			, mManager(manager)
		{
		}

		template<typename ComponentType>
		ComponentType* addComponent()
		{
			return mManager.template addComponent<ComponentType>(mEntity);
		}

		template<typename ComponentType>
		void removeComponent()
		{
			mManager.template removeComponent<ComponentType>(mEntity);
		}

		template<typename... Components>
		std::tuple<Components*...> getComponents()
		{
			return mManager.template getEntityComponents<Components...>(mEntity);
		}

		template<typename ComponentType>
		ComponentType* scheduleAddComponent()
		{
			return mManager.template scheduleAddComponent<ComponentType>(mEntity);
		}

		template<typename ComponentType>
		void scheduleRemoveComponent()
		{
			mManager.template scheduleRemoveComponent<ComponentType>(mEntity);
		}

		[[nodiscard]] Entity getEntity() const
		{
			return mEntity;
		}
		EntityManager& getManager() { return mManager; }

	private:
		Entity mEntity;
		EntityManager& mManager;
	};

} // namespace Ecs
