#pragma once

#include <random>
#include <unordered_set>

#include "Entity.h"
#include "ErrorHandling.h"

namespace Ecs
{
	class EntityGenerator
	{
	public:
		using EntityId = Entity::EntityId;

		EntityGenerator(unsigned int seed)
			: mGenerator(static_cast<std::mt19937::result_type>(seed))
		{}

		~EntityGenerator()
		{
#ifdef ECS_DEBUG_CHECKS_ENABLED
			if (!mUsedIds.empty())
			{
				gErrorHandler("EntityGenerator should be destroyed last and all the users should unregister their entities before");
			}
#endif // ECS_DEBUG_CHECKS_ENABLED
		}

		EntityGenerator(EntityGenerator&) = delete;
		EntityGenerator& operator=(EntityGenerator&) = delete;
		EntityGenerator(EntityGenerator&&) = delete;
		EntityGenerator& operator=(EntityGenerator&&) = delete;

		/**
		 * @brief Generates an unique Entity ID and saves it to avoid collisions in future
		 * @return ID that should be used to for an Enitiy
		 *
		 * Need to call unregisterEntityId when this entity is not being used anymore
		 */
		[[nodiscard]] EntityId generateAndRegisterEntityId()
		{
#ifdef ECS_DEBUG_CHECKS_ENABLED
			const int EntityInsertionTrialsBeforeWarning = 10;
			int insertionTrial = 0;
#endif // ECS_DEBUG_CHECKS_ENABLED

			// should always return a valid entity
			while (true)
			{
#ifdef ECS_DEBUG_CHECKS_ENABLED
				if (insertionTrial == EntityInsertionTrialsBeforeWarning)
				{
					gErrorHandler("Can't generate unique ID for an entity");
				}
				++insertionTrial;
#endif // ECS_DEBUG_CHECKS_ENABLED

				EntityId newId = static_cast<EntityId>(mGenerator());
				auto insertionResult = mUsedIds.insert(newId);
				if (insertionResult.second)
				{
					return newId;
				}
			}
		}

		/**
		 * @brief Generates yet unused id but not register it to save from future collisions
		 * @return unused Entity ID
		 *
		 * Need to be used together with registerEntityId, can produce collisions if between
		 * call to this function and call to registerEntityId any other calls happened that may have
		 * generated new Entity ID.
		 */
		[[nodiscard]] EntityId generateEntityId()
		{
			while (true)
			{
				EntityId newId = static_cast<EntityId>(mGenerator());
				if (mUsedIds.find(newId) == mUsedIds.end())
				{
					return newId;
				}
			}
		}

		/**
		 * @brief Registers an existing Entity ID to avoid generating this entity again
		 * @param entityId  Entity ID that should be registered
		 * @return true if the registration was successful, false if this entity has been
		 *         already registered
		 */
		bool registerEntityId(EntityId entityId)
		{
			auto insertionResult = mUsedIds.insert(entityId);
			return insertionResult.second;
		}

		void unregisterEntityId(EntityId entityId)
		{
			[[maybe_unused]] size_t erasedCount = mUsedIds.erase(entityId);
#ifdef ECS_DEBUG_CHECKS_ENABLED
			if (erasedCount == 0)
			{
				gErrorHandler("Erased an entity that was never created");
			}
#endif // ECS_DEBUG_CHECKS_ENABLED
		}

	private:
		std::unordered_set<EntityId> mUsedIds;
		std::mt19937 mGenerator;
	};
} // namespace Ecs
