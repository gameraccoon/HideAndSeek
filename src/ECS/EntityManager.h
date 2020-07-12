#pragma once

#include <tuple>
#include <unordered_map>

#include "Base/Types/TemplateAliases.h"

#include "ECS/Component.h"
#include "ECS/Entity.h"
#include "ECS/Delegates.h"
#include "ECS/ComponentMap.h"

struct ComponentSerializersHolder;
class JsonComponentSerializationHolder;

class EntityManager
{
public:
	EntityManager() = default;
	~EntityManager();

	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;
	EntityManager(EntityManager&&) = delete;
	EntityManager& operator=(EntityManager&&) = delete;

	Entity addEntity();
	void removeEntity(Entity entity);
	std::unordered_map<Entity::EntityID, size_t>& getEntities() { return mEntityIndexMap; }

	// these two should be used carefully (added for the editor)
	Entity getNonExistentEntity();
	void insertEntityUnsafe(Entity entity);

	void getAllEntityComponents(Entity entity, std::vector<BaseComponent*>& outComponents);

	template<typename ComponentType>
	bool doesEntityHaveComponent(Entity entity)
	{
		auto entityIdxItr = mEntityIndexMap.find(entity.getID());
		if (entityIdxItr == mEntityIndexMap.end())
		{
			return false;
		}

		auto [componentVector] = mComponents.getComponentVectors<ComponentType>();

		return entityIdxItr->second < componentVector.size() && componentVector[entityIdxItr->second] != nullptr;
	}

	template<typename ComponentType>
	ComponentType* addComponent(Entity entity)
	{
		auto entityIdxItr = mEntityIndexMap.find(entity.getID());
		if (entityIdxItr == mEntityIndexMap.end())
		{
			return nullptr;
		}

		ComponentType* component = new ComponentType();

		addComponentToEntity(entityIdxItr->second, component, ComponentType::GetTypeName());

		return component;
	}

	void addComponent(Entity entity, BaseComponent* component, StringID typeID);

	template<typename ComponentType>
	void removeComponent(Entity entity)
	{
		removeComponent(entity, ComponentType::GetTypeName());
	}

	void removeComponent(Entity entity, StringID typeID);

	template<typename ComponentType>
	ComponentType* scheduleAddComponent(Entity entity)
	{
		ComponentType* component = new ComponentType();
		scheduleAddComponentToEntity(entity, component, ComponentType::GetTypeName());
		return component;
	}

	void scheduleAddComponentToEntity(Entity entity, BaseComponent* component, StringID typeID);

	template<typename ComponentType>
	void scheduleRemoveComponent(Entity entity)
	{
		scheduleRemoveComponent(entity, ComponentType::GetTypeName());
	}

	void scheduleRemoveComponent(Entity entity, StringID typeID);
	void executeScheduledActions();

	template<typename... Components>
	std::tuple<Components*...> getEntityComponents(Entity entity)
	{
		auto entityIdxItr = mEntityIndexMap.find(entity.getID());
		if (entityIdxItr == mEntityIndexMap.end())
		{
			return getEmptyComponents<Components...>();
		}
		EntityIndex entityIdx = entityIdxItr->second;

		auto componentVectors = mComponents.getComponentVectors<Components...>();
		return getEntityComponentSet<Components...>(entityIdx, componentVectors);
	}

	template<typename FirstComponent, typename... Components, typename... AdditionalData>
	void getComponents(TupleVector<AdditionalData..., FirstComponent*, Components*...>& inOutComponents, AdditionalData... data)
	{
		auto componentVectors = mComponents.getComponentVectors<FirstComponent, Components...>();
		auto& firstComponentVector = std::get<0>(componentVectors);
		size_t shortestVectorSize = getShortestVector(componentVectors);

		constexpr unsigned componentsSize = sizeof...(Components);

		for (EntityIndex entityIndex = 0, i_size = shortestVectorSize; entityIndex < i_size; ++entityIndex)
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
		auto componentVectors = mComponents.getComponentVectors<FirstComponent, Components...>();
		auto& firstComponentVector = std::get<0>(componentVectors);
		size_t shortestVectorSize = getShortestVector(componentVectors);

		constexpr unsigned componentsSize = sizeof...(Components);

		for (auto& [entityID, entityIndex] : mEntityIndexMap)
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
				inOutComponents.push_back(std::tuple_cat(std::make_tuple(Entity(entityID)), std::make_tuple(data...), std::move(components)));
			}
		}
	}

	template<typename FirstComponent, typename... Components, typename FunctionType, typename... AdditionalData>
	void forEachComponentSet(FunctionType processor, AdditionalData... data)
	{
		auto componentVectors = mComponents.getComponentVectors<FirstComponent, Components...>();
		auto& firstComponentVector = std::get<0>(componentVectors);
		size_t shortestVectorSize = getShortestVector(componentVectors);

		constexpr unsigned componentsSize = sizeof...(Components);

		for (EntityIndex entityIndex = 0, i_size = shortestVectorSize; entityIndex < i_size; ++entityIndex)
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
		auto componentVectors = mComponents.getComponentVectors<FirstComponent, Components...>();
		auto& firstComponentVector = std::get<0>(componentVectors);
		size_t shortestVectorSize = getShortestVector(componentVectors);

		constexpr unsigned componentsSize = sizeof...(Components);

		for (auto& [entityID, entityIndex] : mEntityIndexMap)
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

			std::apply(processor, std::tuple_cat(std::make_tuple(Entity(entityID)), std::make_tuple(data...), std::move(components)));
		}
	}

	void getEntitiesHavingComponents(const std::vector<StringID>& componentIndexes, std::vector<Entity>& inOutEntities) const;

	bool hasEntity(Entity entity);

	// methods for the editor
	void getPrefabFromEntity(nlohmann::json& json, Entity entity, const JsonComponentSerializationHolder& jsonSerializationHolder);
	Entity createPrefabInstance(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers);
	void applyPrefabToExistentEntity(const nlohmann::json& json, Entity entity, const ComponentSerializersHolder& componentSerializers);

	void transferEntityTo(EntityManager& otherManager, Entity entity);

	nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers);

	void clearCaches();
	[[nodiscard]] bool hasAnyEntities() const;

public:
	MulticastDelegate<> OnEntityAdded;
	MulticastDelegate<> OnEntityRemoved;

private:
	using EntityIndex = size_t;

	struct ComponentToAdd
	{
		Entity entity;
		BaseComponent* component;
		StringID typeID;

		ComponentToAdd(Entity entity, BaseComponent* component, StringID typeID)
			: entity(entity)
			, component(component)
			, typeID(typeID)
		{}
	};

	struct ComponentToRemove
	{
		Entity entity;
		StringID typeID;

		ComponentToRemove(Entity entity, StringID typeID)
			: entity(entity)
			, typeID(typeID)
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
	static size_t getShortestVector(const std::tuple<ComponentVector&...>& vectorTuple)
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

	void addComponentToEntity(EntityIndex entityIdx, BaseComponent* component, StringID typeID);

private:
	ComponentMap mComponents;
	std::unordered_map<Entity::EntityID, EntityIndex> mEntityIndexMap;
	std::unordered_map<EntityIndex, Entity::EntityID> mIndexEntityMap;

	std::vector<ComponentToAdd> mScheduledComponentAdditions;
	std::vector<ComponentToRemove> mScheduledComponentRemovements;

	EntityIndex mNextEntityIndex = 0;
};
