#include "Base/precomp.h"

#include "ECS/EntityManager.h"

#include <algorithm>
#include <ranges>

#include <nlohmann/json.hpp>

#include <soasort.h>

#include "Base/Random/Random.h"

#include "ECS/ComponentFactory.h"
#include "ECS/Serialization/ComponentSerializersHolder.h"

static const int EntityInsertionTrialsLimit = 10;

EntityManager::~EntityManager()
{
	for (auto& componentVector : mComponents)
	{
		for (auto component : componentVector.second)
		{
			delete component;
		}
	}
}

Entity EntityManager::addEntity()
{
	int insertionTrial = 0;

	while (insertionTrial < EntityInsertionTrialsLimit)
	{
		Entity::EntityID id = static_cast<Entity::EntityID>(Random::GlobalGenerator());
		auto insertionResult = mEntityIndexMap.try_emplace(id, mNextEntityIndex);
		if (insertionResult.second)
		{
			mIndexEntityMap.emplace(mNextEntityIndex, id);
			++mNextEntityIndex;
			OnEntityAdded.broadcast();
			return Entity(id);
		}
		++insertionTrial;
	}

	ReportError("Can't generate unique ID for an entity");
	return Entity(0);
}

Entity EntityManager::getNonExistentEntity()
{
	int generationTrial = 0;

	while (generationTrial < EntityInsertionTrialsLimit)
	{
		Entity::EntityID id = static_cast<Entity::EntityID>(Random::GlobalGenerator());
		if (mEntityIndexMap.find(id) == mEntityIndexMap.end())
		{
			return Entity(id);
		}
		++generationTrial;
	}

	ReportError("Can't generate unique ID for an entity");
	return Entity(0);
}

void EntityManager::insertEntityUnsafe(Entity entity)
{
	mEntityIndexMap.emplace(entity.getID(), mNextEntityIndex);
	mIndexEntityMap.emplace(mNextEntityIndex, entity.getID());
	++mNextEntityIndex;

	OnEntityAdded.broadcast();
}

void EntityManager::removeEntity(Entity entity)
{
	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr == mEntityIndexMap.end())
	{
		return;
	}
	EntityIndex oldEntityIdx = entityIdxItr->second;

	--mNextEntityIndex; // now it points to the element that going to be removed

	for (auto& componentVector : mComponents)
	{
		// if the vector containts deleted entity
		if (oldEntityIdx < componentVector.second.size())
		{
			// remove the element
			delete componentVector.second[oldEntityIdx];
			componentVector.second[oldEntityIdx] = nullptr;

			// if the vector containts the last entity
			if (mNextEntityIndex < componentVector.second.size() && oldEntityIdx != mNextEntityIndex)
			{
				// move it to the freed space
				std::swap(componentVector.second[oldEntityIdx], componentVector.second[mNextEntityIndex]);
			}
		}
	}

	mEntityIndexMap.erase(entity.getID());

	if (oldEntityIdx != mNextEntityIndex)
	{
		// relink maps
		Entity::EntityID entityID = mIndexEntityMap[mNextEntityIndex];
		mEntityIndexMap[entityID] = oldEntityIdx;
		mIndexEntityMap[oldEntityIdx] = entityID;
	}
	mIndexEntityMap.erase(mNextEntityIndex);

	OnEntityRemoved.broadcast();
}

void EntityManager::getAllEntityComponents(Entity entity, std::vector<BaseComponent*>& outComponents)
{
	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr != mEntityIndexMap.end())
	{
		EntityIndex index = entityIdxItr->second;
		for (auto& componentArray : mComponents)
		{
			if (componentArray.second.size() > index && componentArray.second[index] != nullptr)
			{
				outComponents.push_back(componentArray.second[index]);
			}
		}
	}
}

bool EntityManager::doesEntityHaveComponent(Entity entity, StringID typeID)
{
	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr != mEntityIndexMap.end())
	{
		std::vector<BaseComponent*>& componentArray = mComponents.getComponentVectorByID(typeID);
		EntityIndex index = entityIdxItr->second;
		return (componentArray.size() > index && componentArray[index] != nullptr);
	}
	return false;
}

void EntityManager::addComponent(Entity entity, BaseComponent* component, StringID typeID)
{
	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr == mEntityIndexMap.end())
	{
		return;
	}

	addComponentToEntity(entityIdxItr->second, component, typeID);
}

void EntityManager::removeComponent(Entity entity, StringID typeID)
{
	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr == mEntityIndexMap.end())
	{
		return;
	}

	auto& componentsVector = mComponents.getComponentVectorByID(typeID);

	if (componentsVector.size() > entityIdxItr->second)
	{
		delete componentsVector[entityIdxItr->second];
		componentsVector[entityIdxItr->second] = nullptr;
	}
}

void EntityManager::scheduleAddComponentToEntity(Entity entity, BaseComponent* component, StringID typeID)
{
	mScheduledComponentAdditions.emplace_back(entity, component, typeID);
}

void EntityManager::scheduleRemoveComponent(Entity entity, StringID typeID)
{
	mScheduledComponentRemovements.emplace_back(entity, typeID);
}

void EntityManager::executeScheduledActions()
{
	for (const auto& addition : mScheduledComponentAdditions)
	{
		addComponent(addition.entity, addition.component, addition.typeID);
	}
	mScheduledComponentAdditions.clear();

	for (const auto& removement : mScheduledComponentRemovements)
	{
		removeComponent(removement.entity, removement.typeID);
	}
	mScheduledComponentRemovements.clear();
}

void EntityManager::getEntitiesHavingComponents(const std::vector<StringID>& componentIndexes, std::vector<Entity>& inOutEntities) const
{
	if (componentIndexes.empty())
	{
		return;
	}

	EntityIndex endIdx = std::numeric_limits<EntityIndex>::max();
	std::vector<const std::vector<BaseComponent*>*> componentVectors;
	componentVectors.reserve(componentIndexes.size());
	for (StringID typeID : componentIndexes)
	{
		auto& componentVector = mComponents.getComponentVectorByID(typeID);

		endIdx = std::min(endIdx, componentVector.size());

		componentVectors.push_back(&componentVector);
	}

	for (EntityIndex idx = 0; idx < endIdx; ++idx)
	{
		bool hasAllComponents = std::all_of(
			componentVectors.cbegin(),
			componentVectors.cend(),
			[idx](const std::vector<BaseComponent*>* componentVector){ return (*componentVector)[idx] != nullptr; }
		);

		if (hasAllComponents)
		{
			inOutEntities.emplace_back(mIndexEntityMap.find(idx)->second);
		}
	}
}

bool EntityManager::hasEntity(Entity entity)
{
	return mEntityIndexMap.find(entity.getID()) != mEntityIndexMap.end();
}

void EntityManager::getPrefabFromEntity(nlohmann::json& json, Entity entity, const JsonComponentSerializationHolder& jsonSerializationHolder)
{
	std::vector<BaseComponent*> components;
	getAllEntityComponents(entity, components);

	for (BaseComponent* component : components)
	{
		auto componentObj = nlohmann::json{};
		StringID componentTypeName = component->getComponentTypeName();
		jsonSerializationHolder.getComponentSerializerFromClassName(componentTypeName)->toJson(componentObj, component);
		json[ID_TO_STR(componentTypeName)] = componentObj;
	}
}

Entity EntityManager::createPrefabInstance(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
{
	Entity entity = addEntity();
	applyPrefabToExistentEntity(json, entity, componentSerializers);
	return entity;
}

void EntityManager::applyPrefabToExistentEntity(const nlohmann::json& json, Entity entity, const ComponentSerializersHolder& componentSerializers)
{
	for (const auto& [componentTypeNameStr, componentObj] : json.items())
	{
		StringID componentTypeName = STR_TO_ID(componentTypeNameStr);
		BaseComponent* component = componentSerializers.factory.createComponent(componentTypeName);

		componentSerializers.jsonSerializer.getComponentSerializerFromClassName(componentTypeName)->fromJson(componentObj, component);

		addComponent(
			entity,
			component,
			componentTypeName
		);
	}
}

void EntityManager::transferEntityTo(EntityManager& otherManager, Entity entity)
{
	AssertFatal(this != &otherManager, "Transferring entity to the same manager. This should never hapen");

	auto entityIdxItr = mEntityIndexMap.find(entity.getID());
	if (entityIdxItr == mEntityIndexMap.end())
	{
		return;
	}

	// ToDo use global entity ID collision detection
	MAYBE_UNUSED auto insertionResult = otherManager.mEntityIndexMap.try_emplace(entity.getID(), otherManager.mNextEntityIndex);
	AssertFatal(insertionResult.second, "EntityID is not unique, two entities have just collided");
	otherManager.mIndexEntityMap.emplace(otherManager.mNextEntityIndex, entity.getID());
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

			// if the vector containts the last entity
			if (mNextEntityIndex < componentVector.second.size() && oldEntityIdx != mNextEntityIndex)
			{
				// move it to the freed space
				std::swap(componentVector.second[oldEntityIdx], componentVector.second[mNextEntityIndex]);
			}
		}
	}

	mEntityIndexMap.erase(entity.getID());

	if (oldEntityIdx != mNextEntityIndex)
	{
		// relink maps
		Entity::EntityID entityID = mIndexEntityMap[mNextEntityIndex];
		mEntityIndexMap[entityID] = oldEntityIdx;
		mIndexEntityMap[oldEntityIdx] = entityID;
	}
	mIndexEntityMap.erase(mNextEntityIndex);
}

nlohmann::json EntityManager::toJson(const ComponentSerializersHolder& componentSerializers) const
{
	std::vector<std::pair<Entity::EntityID, EntityIndex>> sortedEntityIndexMap;
	sortedEntityIndexMap.reserve(mEntityIndexMap.size());
	for (const auto& indexPair : mEntityIndexMap)
	{
		sortedEntityIndexMap.push_back(indexPair);
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
			auto componenObj = nlohmann::json{};
			if (component != nullptr)
			{
				jsonSerializer->toJson(componenObj, component);
			}
			componentArrayObject.push_back(componenObj);
		}
		components[ID_TO_STR(componentArray.first)] = componentArrayObject;
	}
	outJson["components"] = components;

	return outJson;
}

void EntityManager::fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers)
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
		StringID type = STR_TO_ID(typeStr);
		ComponentFactory::CreationFn componentCreateFn = componentSerializers.factory.getCreationFn(type);
		if (componentCreateFn != nullptr)
		{
			const JsonComponentSerializer* jsonSerializer = componentSerializers.jsonSerializer.getComponentSerializerFromClassName(type);
			std::vector<BaseComponent*>& componentsVector = mComponents.getOrCreateComponentVectorByID(type);
			componentsVector.reserve(vector.size());
			for (const auto& componentData : vector)
			{
				if (!componentData.is_null())
				{
					BaseComponent* component = componentCreateFn();
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

void EntityManager::stableSortEntitiesByID()
{
	std::vector<Entity::EntityID> ids;
	ids.resize(mNextEntityIndex);
	for (auto [entityID, idx] : mEntityIndexMap)
	{
		ids[idx] = entityID;
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
		Entity::EntityID id = ids[idx];
		mEntityIndexMap[id] = idx;
		mIndexEntityMap[idx] = id;
	}
}

void EntityManager::clearCaches()
{
	for (auto& componentVectorPair : mComponents)
	{
		auto& componentVector = componentVectorPair.second;
		auto lastFilledRIt = std::find_if(componentVector.rbegin(), componentVector.rend(),
			[](const BaseComponent* component){ return component != nullptr; }
		);

		if (lastFilledRIt != componentVector.rend())
		{
			size_t lastFilledIdx = std::distance(lastFilledRIt, componentVector.rend());
			componentVector.erase(componentVector.begin() + lastFilledIdx, componentVector.end());
		}
		else
		{
			componentVector.clear();
		}
	}

	mComponents.cleanEmptyVetors();
}

bool EntityManager::hasAnyEntities() const
{
	return !mEntityIndexMap.empty();
}

void EntityManager::addComponentToEntity(EntityIndex entityIdx, BaseComponent* component, StringID typeID)
{
	auto& componentsVector = mComponents.getOrCreateComponentVectorByID(typeID);
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
