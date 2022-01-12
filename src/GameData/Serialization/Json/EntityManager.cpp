#include "Base/precomp.h"

#include "GameData/Serialization/Json/EntityManager.h"

#include <nlohmann/json.hpp>
#include <soasort.h>

namespace Json
{
	static void StableSortEntitiesById(RaccoonEcs::ComponentMapImpl<StringId>& components, std::vector<Entity>& entities, std::unordered_map<Entity::EntityId, size_t>& entityIndexMap)
	{
		std::vector<size_t> positions;
		soasort::getSortedPositions(positions, entities);

		std::vector<soasort::Swap> swaps;
		soasort::generateSwaps(swaps, positions);

		for (auto& componentVectorPair : components)
		{
			componentVectorPair.second.resize(entities.size(), nullptr);
			soasort::applySwaps(componentVectorPair.second, swaps);
		}

		soasort::applySwaps(entities, swaps);
		for (EntityManager::EntityIndex idx = 0u; idx < entities.size(); ++idx)
		{
			entityIndexMap[entities[idx].getId()] = idx;
		}
	}

	nlohmann::json SerializeEntityManager(EntityManager& entityManager, const Json::ComponentSerializationHolder& jsonSerializationHolder)
	{
		entityManager.applySortingFunction(&StableSortEntitiesById);
		entityManager.clearCaches();

		// we sorted them two lines above
		const std::vector<Entity>& sortedEntities = entityManager.getEntities();
		nlohmann::json entitiesJson;
		for (Entity entity : sortedEntities)
		{
			entitiesJson.push_back(entity.getId());
		}

		nlohmann::json outJson{
			{"entities", entitiesJson}
		};

		auto components = nlohmann::json{};

		for (auto& componentArray : entityManager.getComponentsData())
		{
			auto componentArrayObject = nlohmann::json::array();
			if (const ComponentSerializer* jsonSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(componentArray.first))
			{
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
			outEntityManager.addExistingEntityUnsafe(entity);
			entities.push_back(entity);
		}

		const auto& components = json.at("components");
		for (const auto& [typeStr, vector] : components.items())
		{
			StringId type = STR_TO_ID(typeStr);
			if (const ComponentSerializer* jsonSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(type))
			{
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
			else
			{
				ReportFatalError("Unknown component %s", type);
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
			if (const ComponentSerializer* componentSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName))
			{
				componentSerializer->toJson(componentObj, componentData.component);
				json[ID_TO_STR(componentData.typeId)] = componentObj;
			}
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
			if (const ComponentSerializer* componentSerializer = jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName))
			{
				componentSerializer->fromJson(componentObj, component);
			}
		}
	}
}
