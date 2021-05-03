#pragma once

#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <ranges>

#include <nlohmann/json.hpp>

#include <soasort.h>

#include "Base/Types/TemplateAliases.h"
#include "Base/Random/Random.h"
#include "Base/Types/String/StringId.h"

#include "ECS/Entity.h"
#include "ECS/Delegates.h"
#include "ECS/ComponentMap.h"
#include "ECS/ComponentFactory.h"
#include "ECS/Serialization/ComponentSerializersHolder.h"
#include "ECS/TypedComponent.h"

namespace Ecs
{
	template <typename ComponentTypeId>
	class EntityManagerImpl
	{
	public:
		using EntityManager = EntityManagerImpl<ComponentTypeId>;
		using TypedComponent = TypedComponentImpl<ComponentTypeId>;
		using ComponentMap = ComponentMapImpl<ComponentTypeId>;
		using ComponentFactory = ComponentFactoryImpl<ComponentTypeId>;

		EntityManagerImpl(const ComponentFactory& componentFactory)
			: mComponentFactory(componentFactory)
		{}

		~EntityManagerImpl()
		{
			for (auto& componentVector : mComponents)
			{
				auto deleterFn = mComponentFactory.getDeletionFn(componentVector.first);

				for (auto component : componentVector.second)
				{
					deleterFn(component);
				}
			}
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

			ReportError("Can't generate unique ID for an entity");
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

		[[nodiscard]] const std::unordered_map<Entity::EntityId, size_t>& getEntities() const { return mEntityIndexMap; }

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

			ReportError("Can't generate unique ID for an entity");
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
		void getComponents(TupleVector<AdditionalData..., FirstComponent*, Components*...>& inOutComponents, AdditionalData... data)
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
		void getComponentsWithEntities(TupleVector<Entity, AdditionalData..., FirstComponent*, Components*...>& inOutComponents, AdditionalData... data)
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
			for (StringId typeId : componentIndexes)
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

		// methods for the editor
		void getPrefabFromEntity(nlohmann::json& json, Entity entity, const JsonComponentSerializationHolder& jsonSerializationHolder)
		{
			std::vector<TypedComponent> components;
			getAllEntityComponents(entity, components);

			for (const TypedComponent& componentData : components)
			{
				auto componentObj = nlohmann::json{};
				StringId componentTypeName = componentData.typeId;
				jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName)->toJson(componentObj, componentData.component);
				json[ID_TO_STR(componentData.typeId)] = componentObj;
			}
		}

		Entity createPrefabInstance(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
		{
			Entity entity = addEntity();
			applyPrefabToExistentEntity(json, entity, componentSerializers);
			return entity;
		}

		void applyPrefabToExistentEntity(const nlohmann::json& json, Entity entity, const ComponentSerializersHolder& componentSerializers)
		{
			for (const auto& [componentTypeNameStr, componentObj] : json.items())
			{
				ComponentTypeId componentTypeName = STR_TO_ID(componentTypeNameStr);
				void* component = mComponentFactory.createComponent(componentTypeName);

				componentSerializers.jsonSerializer.getComponentSerializerFromClassName(componentTypeName)->fromJson(componentObj, component);

				addComponent(
					entity,
					component,
					componentTypeName
				);
			}
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
			MAYBE_UNUSED auto insertionResult = otherManager.mEntityIndexMap.try_emplace(entity.getId(), otherManager.mNextEntityIndex);
			AssertFatal(insertionResult.second, "EntityId is not unique, two entities have just collided");
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

		[[nodiscard]] nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const
		{
			std::vector<std::pair<Entity::EntityId, EntityIndex>> sortedEntityIndexMap;
			sortedEntityIndexMap.reserve(mEntityIndexMap.size());
			for (const auto& indexPair : mEntityIndexMap)
			{
				sortedEntityIndexMap.emplace_back(indexPair);
			}

			std::ranges::sort(sortedEntityIndexMap);

			nlohmann::json outJson{
				{"entityIndexMap", sortedEntityIndexMap}
			};

			auto components = nlohmann::json{};

			for (auto& componentArray : mComponents)
			{
				auto componentArrayObject = nlohmann::json::array();
				const JsonComponentSerializer* jsonSerializer = componentSerializers.jsonSerializer.getComponentSerializerFromClassName(componentArray.first);
				for (auto& component : componentArray.second)
				{
					auto componentObj = nlohmann::json{};
					if (component != nullptr)
					{
						jsonSerializer->toJson(componentObj, component);
					}
					componentArrayObject.push_back(componentObj);
				}
				components[ID_TO_STR(componentArray.first)] = componentArrayObject;
			}
			outJson["components"] = components;

			return outJson;
		}

		void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
		{
			json.at("entityIndexMap").get_to(mEntityIndexMap);

			for (const auto& item : mEntityIndexMap)
			{
				mIndexEntityMap[item.second] = item.first;
			}

			auto maxElementIt = std::max_element(mIndexEntityMap.begin(), mIndexEntityMap.end());
			if (maxElementIt != mIndexEntityMap.end())
			{
				mNextEntityIndex = maxElementIt->first + 1;
			}
			else
			{
				mNextEntityIndex = 0;
			}

			const auto& components = json.at("components");
			for (const auto& [typeStr, vector] : components.items())
			{
				StringId type = STR_TO_ID(typeStr);
				typename ComponentFactory::CreationFn componentCreateFn = mComponentFactory.getCreationFn(type);
				if (componentCreateFn != nullptr)
				{
					const JsonComponentSerializer* jsonSerializer = componentSerializers.jsonSerializer.getComponentSerializerFromClassName(type);
					std::vector<void*>& componentsVector = mComponents.getOrCreateComponentVectorById(type);
					componentsVector.reserve(vector.size());
					for (const auto& componentData : vector)
					{
						if (!componentData.is_null())
						{
							void* component = componentCreateFn();
							jsonSerializer->fromJson(componentData, component);
							componentsVector.push_back(component);
						}
						else
						{
							componentsVector.push_back(nullptr);
						}
					}
				}
			}
		}

		// helper functions for cleanup before saving
		void stableSortEntitiesById()
		{
			std::vector<Entity::EntityId> ids;
			ids.resize(mNextEntityIndex);
			for (auto [entityId, idx] : mEntityIndexMap)
			{
				ids[idx] = entityId;
			}

			std::vector<size_t> positions;
			soasort::getSortedPositions(positions, ids);

			std::vector<soasort::Swap> swaps;
			soasort::generateSwaps(swaps, positions);

			for (auto& componentVectorPair : mComponents)
			{
				componentVectorPair.second.resize(mNextEntityIndex, nullptr);
				soasort::applySwaps(componentVectorPair.second, swaps);
			}

			soasort::applySwaps(ids, swaps);
			for (EntityIndex idx = 0u; idx < mNextEntityIndex; ++idx)
			{
				Entity::EntityId id = ids[idx];
				mEntityIndexMap[id] = idx;
				mIndexEntityMap[idx] = id;
			}
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

		[[nodiscard]] bool hasAnyEntities() const
		{
			return !mEntityIndexMap.empty();
		}

	public:
		MulticastDelegate<> onEntityAdded;
		MulticastDelegate<> onEntityRemoved;

	private:
		using EntityIndex = size_t;

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
			else
			{
				ReportFatalError("Trying to add a component when the entity already has one of the same type. This will result in memory leak");
			}
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
