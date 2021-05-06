#include "Base/precomp.h"

#include "GameData/Serialization/Json/EntityManager.h"

#include <nlohmann/json.hpp>
#include <soasort.h>

namespace Json
{
	static void StableSortEntitiesById(EntityManager& entityManager)
	{
		std::vector<Entity::EntityId> ids;

		auto [components, entityIndexMap, indexEntityMap] = entityManager.getSortableData();

		ids.resize(entityIndexMap.size());
		for (auto [entityId, idx] : entityIndexMap)
		{
			ids[idx] = entityId;
		}

		std::vector<size_t> positions;
		soasort::getSortedPositions(positions, ids);

		std::vector<soasort::Swap> swaps;
		soasort::generateSwaps(swaps, positions);

		for (auto& componentVectorPair : components)
		{
			componentVectorPair.second.resize(ids.size(), nullptr);
			soasort::applySwaps(componentVectorPair.second, swaps);
		}

		soasort::applySwaps(ids, swaps);
		for (EntityManager::EntityIndex idx = 0u; idx < ids.size(); ++idx)
		{
			Entity::EntityId id = ids[idx];
			entityIndexMap[id] = idx;
			indexEntityMap[idx] = id;
		}
	}

	nlohmann::json SerializeEntityManager(EntityManager& entityManager, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		StableSortEntitiesById(entityManager);

		entityManager.clearCaches();

		std::vector<Entity::EntityId> sortedEntities;
		const auto& entityIndexMap = entityManager.getEntities();
		sortedEntities.reserve(entityIndexMap.size());
		for (const auto& indexPair : entityIndexMap)
		{
			sortedEntities.emplace_back(indexPair.first);
		}

		std::ranges::sort(sortedEntities);

		nlohmann::json outJson{
			{"entities", sortedEntities}
		};

		auto components = nlohmann::json{};

		for (auto& componentArray : entityManager.getComponentsData())
		{
			auto componentArrayObject = nlohmann::json::array();
			const ComponentSerializer* jsonSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(componentArray.first);
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

	void DeserializeEntityManager(EntityManager& outEntityManager, const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		// make sure the manager is fully reset
		outEntityManager.clear();

		// entities go in order of indexes, starting from zero
		const auto& entitiesJson = json.at("entities");

		std::vector<Entity> entities;
		entities.reserve(entitiesJson.size());
		for (const auto& entityData : entitiesJson)
		{
			Entity entity(entityData.get<Entity::EntityId>());
			outEntityManager.insertEntityUnsafe(entity);
			entities.push_back(entity);
		}

		const auto& components = json.at("components");
		for (const auto& [typeStr, vector] : components.items())
		{
			StringId type = STR_TO_ID(typeStr);
			const ComponentSerializer* jsonSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(type);
			size_t entityIndex = 0;
			for (const auto& componentData : vector)
			{
				if (!componentData.is_null())
				{
					void* component = outEntityManager.addComponentByType(entities[entityIndex], type);
					jsonSerializer->fromJson(componentData, component);
				}
				++entityIndex;
			}
		}
	}

	void GetPrefabFromEntity(const EntityManager& entityManager, nlohmann::json& json, Entity entity, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		std::vector<ConstTypedComponent> components;
		entityManager.getAllEntityComponents(entity, components);

		for (const ConstTypedComponent& componentData : components)
		{
			auto componentObj = nlohmann::json{};
			StringId componentTypeName = componentData.typeId;
			jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName)->toJson(componentObj, componentData.component);
			json[ID_TO_STR(componentData.typeId)] = componentObj;
		}
	}

	Entity CreatePrefabInstance(EntityManager& entityManager, const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		Entity entity = entityManager.addEntity();
		ApplyPrefabToExistentEntity(entityManager, json, entity, jsonSerializationHolder);
		return entity;
	}

	void ApplyPrefabToExistentEntity(EntityManager& entityManager, const nlohmann::json& json, Entity entity, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		for (const auto& [componentTypeNameStr, componentObj] : json.items())
		{
			StringId componentTypeName = STR_TO_ID(componentTypeNameStr);

			void* component = entityManager.addComponentByType(entity, componentTypeName);
			jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName)->fromJson(componentObj, component);
		}
	}
}
