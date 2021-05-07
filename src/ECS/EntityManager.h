#pragma once

#include <algorithm>
#include <ranges>
#include <tuple>
#include <unordered_map>

#include "Base/Random/Random.h"

#include "ComponentFactory.h"
#include "ComponentMap.h"
#include "Delegates.h"
#include "Entity.h"
#include "ErrorHandling.h"
#include "TypedComponent.h"

namespace Ecs
{
	template <typename ComponentTypeId>
	class EntityManagerImpl
	{
	public:
		using EntityManager = EntityManagerImpl<ComponentTypeId>;
		using TypedComponent = TypedComponentImpl<ComponentTypeId>;
		using ConstTypedComponent = ConstTypedComponentImpl<ComponentTypeId>;
		using ComponentMap = ComponentMapImpl<ComponentTypeId>;
		using ComponentFactory = ComponentFactoryImpl<ComponentTypeId>;

		using EntityIndex = size_t;

	public:
		EntityManagerImpl(const ComponentFactory& componentFactory)
			: mComponentFactory(componentFactory)
		{}

		~EntityManagerImpl()
		{
			clear();
		}

		EntityManagerImpl(const EntityManagerImpl&) = delete;
		EntityManagerImpl& operator=(const EntityManagerImpl&) = delete;
		EntityManagerImpl(EntityManagerImpl&&) = delete;
		EntityManagerImpl& operator=(EntityManagerImpl&&) = delete;

		Entity addEntity()
		{
			const int EntityInsertionTrialsLimit = 10;
			int insertionTrial = 0;

			while (insertionTrial < EntityInsertionTrialsLimit)
			{
				Entity::EntityId id = static_cast<Entity::EntityId>(Random::gGlobalGenerator());
				auto insertionResult = mEntityIndexMap.try_emplace(id, mNextEntityIndex);
				if (insertionResult.second)
				{
					mIndexEntityMap.emplace(mNextEntityIndex, id);
					++mNextEntityIndex;
					onEntityAdded.broadcast();
					return Entity(id);
				}
				++insertionTrial;
			}

#ifdef ECS_DEBUG_CHECKS_ENABLED
			gErrorHandler("Can't generate unique ID for an entity");
#endif // ECS_DEBUG_CHECKS_ENABLED
			return Entity(0);
		}

		void removeEntity(Entity entity)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return;
			}
			EntityIndex oldEntityIdx = entityIdxItr->second;

			--mNextEntityIndex; // now it points to the element that going to be removed

			for (auto& componentVector : mComponents)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(componentVector.first);
				// if the vector contains deleted entity
				if (oldEntityIdx < componentVector.second.size())
				{
					// remove the element
					deleterFn(componentVector.second[oldEntityIdx]);
					componentVector.second[oldEntityIdx] = nullptr;

					// if the vector contains the last entity
					if (mNextEntityIndex < componentVector.second.size() && oldEntityIdx != mNextEntityIndex)
					{
						// move it to the freed space
						std::swap(componentVector.second[oldEntityIdx], componentVector.second[mNextEntityIndex]);
					}
				}
			}

			mEntityIndexMap.erase(entity.getId());

			if (oldEntityIdx != mNextEntityIndex)
			{
				// relink maps
				Entity::EntityId entityID = mIndexEntityMap[mNextEntityIndex];
				mEntityIndexMap[entityID] = oldEntityIdx;
				mIndexEntityMap[oldEntityIdx] = entityID;
			}
			mIndexEntityMap.erase(mNextEntityIndex);

			onEntityRemoved.broadcast();
		}

		[[nodiscard]] const std::unordered_map<Entity::EntityId, EntityIndex>& getEntities() const { return mEntityIndexMap; }

		// these two should be used carefully (added for the editor)
		Entity getNonExistentEntity()
		{
			const int EntityInsertionTrialsLimit = 10;
			int generationTrial = 0;

			while (generationTrial < EntityInsertionTrialsLimit)
			{
				Entity::EntityId id = static_cast<Entity::EntityId>(Random::gGlobalGenerator());
				if (mEntityIndexMap.find(id) == mEntityIndexMap.end())
				{
					return Entity(id);
				}
				++generationTrial;
			}

#ifdef ECS_DEBUG_CHECKS_ENABLED
			gErrorHandler("Can't generate unique ID for an entity");
#endif // ECS_DEBUG_CHECKS_ENABLED
			return Entity(0);
		}

		void insertEntityUnsafe(Entity entity)
		{
			mEntityIndexMap.emplace(entity.getId(), mNextEntityIndex);
			mIndexEntityMap.emplace(mNextEntityIndex, entity.getId());
			++mNextEntityIndex;

			onEntityAdded.broadcast();
		}

		void getAllEntityComponents(Entity entity, std::vector<TypedComponent>& outComponents)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr != mEntityIndexMap.end())
			{
				EntityIndex index = entityIdxItr->second;
				for (auto& componentArray : mComponents)
				{
					if (componentArray.second.size() > index && componentArray.second[index] != nullptr)
					{
						outComponents.emplace_back(componentArray.first, componentArray.second[index]);
					}
				}
			}
		}

		void getAllEntityComponents(Entity entity, std::vector<ConstTypedComponent>& outComponents) const
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr != mEntityIndexMap.end())
			{
				EntityIndex index = entityIdxItr->second;
				for (auto& componentArray : mComponents)
				{
					if (componentArray.second.size() > index && componentArray.second[index] != nullptr)
					{
						outComponents.emplace_back(componentArray.first, componentArray.second[index]);
					}
				}
			}
		}

		bool doesEntityHaveComponent(Entity entity, ComponentTypeId typeId)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr != mEntityIndexMap.end())
			{
				std::vector<void*>& componentArray = mComponents.getComponentVectorById(typeId);
				EntityIndex index = entityIdxItr->second;
				return (componentArray.size() > index && componentArray[index] != nullptr);
			}
			return false;
		}

		template<typename ComponentType>
		bool doesEntityHaveComponent(Entity entity)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return false;
			}

			std::vector<void*> componentVector = mComponents.getComponentVectorById(ComponentType::GetTypeName());

			return entityIdxItr->second < componentVector.size() && componentVector[entityIdxItr->second] != nullptr;
		}

		template<typename ComponentType>
		ComponentType* addComponent(Entity entity)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return nullptr;
			}

			ComponentType* component = HS_NEW ComponentType();

			addComponentToEntity(entityIdxItr->second, component, ComponentType::GetTypeName());

			return component;
		}

		void* addComponentByType(Entity entity, ComponentTypeId typeId)
		{
			auto createFn = mComponentFactory.getCreationFn(typeId);
			void* component = createFn();
			addComponent(entity, component, typeId);
			return component;
		}

		void addComponent(Entity entity, void* component, ComponentTypeId typeId)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return;
			}

			addComponentToEntity(entityIdxItr->second, component, typeId);
		}

		template<typename ComponentType>
		void removeComponent(Entity entity)
		{
			removeComponent(entity, ComponentType::GetTypeName());
		}

		void removeComponent(Entity entity, ComponentTypeId typeId)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return;
			}

			auto& componentsVector = mComponents.getComponentVectorById(typeId);

			if (componentsVector.size() > entityIdxItr->second)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(typeId);
				deleterFn(componentsVector[entityIdxItr->second]);
				componentsVector[entityIdxItr->second] = nullptr;
			}
		}

		template<typename ComponentType>
		ComponentType* scheduleAddComponent(Entity entity)
		{
			ComponentType* component = HS_NEW ComponentType();
			scheduleAddComponentToEntity(entity, component, ComponentType::GetTypeName());
			return component;
		}

		void scheduleAddComponentToEntity(Entity entity, void* component, ComponentTypeId typeId)
		{
			mScheduledComponentAdditions.emplace_back(entity, component, typeId);
		}

		template<typename ComponentType>
		void scheduleRemoveComponent(Entity entity)
		{
			scheduleRemoveComponent(entity, ComponentType::GetTypeName());
		}

		void scheduleRemoveComponent(Entity entity, ComponentTypeId typeId)
		{
			mScheduledComponentRemovements.emplace_back(entity, typeId);
		}

		void executeScheduledActions()
		{
			for (const auto& addition : mScheduledComponentAdditions)
			{
				addComponent(addition.entity, addition.component, addition.typeId);
			}
			mScheduledComponentAdditions.clear();

			for (const auto& removement : mScheduledComponentRemovements)
			{
				removeComponent(removement.entity, removement.typeId);
			}
			mScheduledComponentRemovements.clear();
		}

		template<typename... Components>
		std::tuple<Components*...> getEntityComponents(Entity entity)
		{
			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return getEmptyComponents<Components...>();
			}
			EntityIndex entityIdx = entityIdxItr->second;

			auto componentVectors = mComponents.template getComponentVectors<Components...>();
			return getEntityComponentSet<Components...>(entityIdx, componentVectors);
		}

		template<typename FirstComponent, typename... Components, typename... AdditionalData>
		void getComponents(std::vector<std::tuple<AdditionalData..., FirstComponent*, Components*...>>& inOutComponents, AdditionalData... data)
		{
			auto componentVectors = mComponents.template getComponentVectors<FirstComponent, Components...>();
			auto& firstComponentVector = std::get<0>(componentVectors);
			size_t shortestVectorSize = GetShortestVector(componentVectors);

			constexpr unsigned componentsSize = sizeof...(Components);

			for (EntityIndex entityIndex = 0, iSize = shortestVectorSize; entityIndex < iSize; ++entityIndex)
			{
				auto& firstComponent = firstComponentVector[entityIndex];
				if (firstComponent == nullptr)
				{
					continue;
				}

				auto components = getEntityComponentSet<FirstComponent, Components...>(entityIndex, componentVectors);

				if (std::get<componentsSize>(components) != nullptr)
				{
					inOutComponents.push_back(std::tuple_cat(std::make_tuple(data...), std::move(components)));
				}
			}
		}

		template<typename FirstComponent, typename... Components, typename... AdditionalData>
		void getComponentsWithEntities(std::vector<std::tuple<Entity, AdditionalData..., FirstComponent*, Components*...>>& inOutComponents, AdditionalData... data)
		{
			auto componentVectors = mComponents.template getComponentVectors<FirstComponent, Components...>();
			auto& firstComponentVector = std::get<0>(componentVectors);
			size_t shortestVectorSize = GetShortestVector(componentVectors);

			constexpr unsigned componentsSize = sizeof...(Components);

			for (auto& [entityId, entityIndex] : mEntityIndexMap)
			{
				if (entityIndex >= shortestVectorSize)
				{
					continue;
				}

				auto& firstComponent = firstComponentVector[entityIndex];
				if (firstComponent == nullptr)
				{
					continue;
				}

				auto components = getEntityComponentSet<FirstComponent, Components...>(entityIndex, componentVectors);

				if (std::get<componentsSize>(components) != nullptr)
				{
					inOutComponents.push_back(std::tuple_cat(std::make_tuple(Entity(entityId)), std::make_tuple(data...), std::move(components)));
				}
			}
		}

		template<typename FirstComponent, typename... Components, typename FunctionType, typename... AdditionalData>
		void forEachComponentSet(FunctionType processor, AdditionalData... data)
		{
			auto componentVectors = mComponents.template getComponentVectors<FirstComponent, Components...>();
			auto& firstComponentVector = std::get<0>(componentVectors);
			size_t shortestVectorSize = GetShortestVector(componentVectors);

			constexpr unsigned componentsSize = sizeof...(Components);

			for (EntityIndex entityIndex = 0, iSize = shortestVectorSize; entityIndex < iSize; ++entityIndex)
			{
				auto& firstComponent = firstComponentVector[entityIndex];
				if (firstComponent == nullptr)
				{
					continue;
				}

				auto components = getEntityComponentSet<FirstComponent, Components...>(entityIndex, componentVectors);

				if (std::get<componentsSize>(components) == nullptr)
				{
					continue;
				}

				std::apply(processor, std::tuple_cat(std::make_tuple(data...), std::move(components)));
			}
		}

		template<typename FirstComponent, typename... Components, typename FunctionType, typename... AdditionalData>
		void forEachComponentSetWithEntity(FunctionType processor, AdditionalData... data)
		{
			auto componentVectors = mComponents.template getComponentVectors<FirstComponent, Components...>();
			auto& firstComponentVector = std::get<0>(componentVectors);
			size_t shortestVectorSize = GetShortestVector(componentVectors);

			constexpr unsigned componentsSize = sizeof...(Components);

			for (auto& [entityId, entityIndex] : mEntityIndexMap)
			{
				if (entityIndex >= shortestVectorSize)
				{
					continue;
				}

				auto& firstComponent = firstComponentVector[entityIndex];
				if (firstComponent == nullptr)
				{
					continue;
				}

				auto components = getEntityComponentSet<FirstComponent, Components...>(entityIndex, componentVectors);

				if (std::get<componentsSize>(components) == nullptr)
				{
					continue;
				}

				std::apply(processor, std::tuple_cat(std::make_tuple(Entity(entityId)), std::make_tuple(data...), std::move(components)));
			}
		}

		void getEntitiesHavingComponents(const std::vector<ComponentTypeId>& componentIndexes, std::vector<Entity>& inOutEntities) const
		{
			if (componentIndexes.empty())
			{
				return;
			}

			EntityIndex endIdx = std::numeric_limits<EntityIndex>::max();
			std::vector<const std::vector<void*>*> componentVectors;
			componentVectors.reserve(componentIndexes.size());
			for (ComponentTypeId typeId : componentIndexes)
			{
				auto& componentVector = mComponents.getComponentVectorById(typeId);

				endIdx = std::min(endIdx, componentVector.size());

				componentVectors.push_back(&componentVector);
			}

			for (EntityIndex idx = 0; idx < endIdx; ++idx)
			{
				bool hasAllComponents = std::all_of(
							componentVectors.cbegin(),
							componentVectors.cend(),
							[idx](const std::vector<void*>* componentVector){ return (*componentVector)[idx] != nullptr; }
				);

				if (hasAllComponents)
				{
					inOutEntities.emplace_back(mIndexEntityMap.find(idx)->second);
				}
			}
		}

		bool hasEntity(Entity entity)
		{
			return mEntityIndexMap.find(entity.getId()) != mEntityIndexMap.end();
		}


		void transferEntityTo(EntityManager& otherManager, Entity entity)
		{
			AssertFatal(this != &otherManager, "Transferring entity to the same manager. This should never happen");

			auto entityIdxItr = mEntityIndexMap.find(entity.getId());
			if (entityIdxItr == mEntityIndexMap.end())
			{
				return;
			}

			// ToDo use global entity ID collision detection
			[[maybe_unused]] auto insertionResult = otherManager.mEntityIndexMap.try_emplace(entity.getId(), otherManager.mNextEntityIndex);
#ifdef ECS_DEBUG_CHECKS_ENABLED
			if (!insertionResult.second)
			{
				gErrorHandler("EntityId is not unique, two entities have just collided");
			}
#endif // ECS_DEBUG_CHECKS_ENABLED
			otherManager.mIndexEntityMap.emplace(otherManager.mNextEntityIndex, entity.getId());
			++otherManager.mNextEntityIndex;

			--mNextEntityIndex; // now it points to the element that going to be removed
			EntityIndex oldEntityIdx = entityIdxItr->second;

			for (auto& componentVector : mComponents)
			{
				if (oldEntityIdx < componentVector.second.size())
				{
					// add the element to the new manager
					if (componentVector.second[oldEntityIdx] != nullptr)
					{
						otherManager.addComponent(
									entity,
									componentVector.second[oldEntityIdx],
									componentVector.first
									);
					}

					// remove the element from the old manager
					componentVector.second[oldEntityIdx] = nullptr;

					// if the vector contains the last entity
					if (mNextEntityIndex < componentVector.second.size() && oldEntityIdx != mNextEntityIndex)
					{
						// move it to the freed space
						std::swap(componentVector.second[oldEntityIdx], componentVector.second[mNextEntityIndex]);
					}
				}
			}

			mEntityIndexMap.erase(entity.getId());

			if (oldEntityIdx != mNextEntityIndex)
			{
				// relink maps
				Entity::EntityId entityID = mIndexEntityMap[mNextEntityIndex];
				mEntityIndexMap[entityID] = oldEntityIdx;
				mIndexEntityMap[oldEntityIdx] = entityID;
			}
			mIndexEntityMap.erase(mNextEntityIndex);
		}

		void clearCaches()
		{
			for (auto& componentVectorPair : mComponents)
			{
				auto& componentVector = componentVectorPair.second;
				auto lastFilledRIt = std::find_if(componentVector.rbegin(), componentVector.rend(),
												  [](const void* component){ return component != nullptr; }
				);

				if (lastFilledRIt != componentVector.rend())
				{
					size_t lastFilledIdx = std::distance(lastFilledRIt, componentVector.rend());
					componentVector.erase(componentVector.begin() + static_cast<ptrdiff_t>(lastFilledIdx), componentVector.end());
				}
				else
				{
					componentVector.clear();
				}
			}

			mComponents.cleanEmptyVectors();
		}

		void clear()
		{
			for (auto& componentVector : mComponents)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(componentVector.first);

				for (auto component : componentVector.second)
				{
					deleterFn(component);
				}
				componentVector.second.clear();
			}
			mComponents.cleanEmptyVectors();

			mEntityIndexMap.clear();
			mIndexEntityMap.clear();

			mScheduledComponentAdditions.clear();
			mScheduledComponentRemovements.clear();
			mNextEntityIndex = 0;
		}

		const ComponentMap& getComponentsData() const { return mComponents; }

		auto getSortableData() { return std::make_tuple(std::ref(mComponents), std::ref(mEntityIndexMap), std::ref(mIndexEntityMap)); }

		[[nodiscard]] bool hasAnyEntities() const
		{
			return !mEntityIndexMap.empty();
		}

	public:
		MulticastDelegate<> onEntityAdded;
		MulticastDelegate<> onEntityRemoved;

	private:
		struct ComponentToAdd
		{
			Entity entity;
			void* component;
			ComponentTypeId typeId;

			ComponentToAdd(Entity entity, void* component, ComponentTypeId typeId)
				: entity(entity)
				, component(component)
				, typeId(typeId)
			{}
		};

		struct ComponentToRemove
		{
			Entity entity;
			ComponentTypeId typeId;

			ComponentToRemove(Entity entity, ComponentTypeId typeId)
				: entity(entity)
				, typeId(typeId)
			{}
		};

	private:
		template<int I = 0>
		std::tuple<> getEmptyComponents()
		{
			return std::tuple<>();
		}

		template<typename FirstComponent, typename... Components>
		std::tuple<FirstComponent*, Components*...> getEmptyComponents()
		{
			return std::tuple_cat(std::tuple<FirstComponent*>(nullptr), getEmptyComponents<Components...>());
		}

		template<unsigned Index, typename Datas>
		std::tuple<> getEntityComponentSetInner(EntityIndex /*entityIdx*/, Datas& /*componentVectors*/)
		{
			return std::tuple<>();
		}

		template<unsigned Index, typename Datas, typename FirstComponent, typename... Components>
		std::tuple<FirstComponent*, Components*...> getEntityComponentSetInner(EntityIndex entityIdx, Datas& componentVectors)
		{
			if (std::get<Index>(componentVectors).size() <= entityIdx)
			{
				return getEmptyComponents<FirstComponent, Components...>();
			}

			auto& component = std::get<Index>(componentVectors)[entityIdx];
			if (component == nullptr)
			{
				return getEmptyComponents<FirstComponent, Components...>();
			}

			return std::tuple_cat(std::make_tuple(static_cast<FirstComponent*>(component)), getEntityComponentSetInner<Index + 1, Datas, Components...>(entityIdx, componentVectors));
		}

		template<typename FirstComponent, typename... Components, typename... Data>
		std::tuple<FirstComponent*, Components*...> getEntityComponentSet(EntityIndex entityIdx, std::tuple<std::vector<Data*>&...>& componentVectors)
		{
			using Datas = std::tuple<std::vector<Data*>&...>;
			return getEntityComponentSetInner<0, Datas, FirstComponent, Components...>(entityIdx, componentVectors);
		}

		template<typename... ComponentVector>
		static size_t GetShortestVector(const std::tuple<ComponentVector&...>& vectorTuple)
		{
			size_t minimalSize = std::numeric_limits<size_t>::max();
			std::apply(
				[&minimalSize](const ComponentVector&... componentVector)
				{
					((minimalSize = std::min(minimalSize, componentVector.size())), ...);
				},
				vectorTuple
			);
			return minimalSize;
		}

		void addComponentToEntity(EntityIndex entityIdx, void* component, ComponentTypeId typeId)
		{
			auto& componentsVector = mComponents.getOrCreateComponentVectorById(typeId);
			if (componentsVector.size() <= entityIdx)
			{
				if (componentsVector.capacity() <= entityIdx)
				{
					componentsVector.reserve((entityIdx + 1) * 2);
				}
				componentsVector.resize(entityIdx + 1);
			}

			if (componentsVector[entityIdx] == nullptr)
			{
				componentsVector[entityIdx] = component;
			}
#ifdef ECS_DEBUG_CHECKS_ENABLED
			else
			{
				gErrorHandler("Trying to add a component when the entity already has one of the same type. This will result in memory leak");
			}
#endif // ECS_DEBUG_CHECKS_ENABLED
		}

	private:
		ComponentMap mComponents;
		std::unordered_map<Entity::EntityId, EntityIndex> mEntityIndexMap;
		std::unordered_map<EntityIndex, Entity::EntityId> mIndexEntityMap;

		std::vector<ComponentToAdd> mScheduledComponentAdditions;
		std::vector<ComponentToRemove> mScheduledComponentRemovements;

		EntityIndex mNextEntityIndex = 0;

		// it's a temporary solution to store it here
		const ComponentFactory& mComponentFactory;
	};

} // namespace Ecs
