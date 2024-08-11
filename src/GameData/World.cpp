#include "EngineCommon/precomp.h"

#include "GameData/World.h"

#include <nlohmann/json.hpp>

#include "GameData/Components/SpatialTrackComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"
#include "GameData/Serialization/Json/EntityManager.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

World::World(const ComponentFactory& componentFactory)
	: mEntityManager(componentFactory)
	, mWorldComponents(componentFactory)
	, mSpatialData(componentFactory)
{
}

nlohmann::json World::toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	return nlohmann::json{
		{ "entity_manager", SerializeEntityManager(mEntityManager, jsonSerializerHolder) },
		{ "world_components", SerializeComponentSetHolder(mWorldComponents, jsonSerializerHolder) },
		{ "spatial_data", mSpatialData.toJson(jsonSerializerHolder) }
	};
}

static void InitSpatialTrackedEntities(SpatialWorldData& spatialData, ComponentSetHolder& worldComponents)
{
	auto [trackedSpatialEntities] = worldComponents.getComponents<TrackedSpatialEntitiesComponent>();

	auto& cells = spatialData.getAllCells();
	for (auto& [_, cell] : cells)
	{
		cell.getEntityManager().forEachComponentSetWithEntity<const SpatialTrackComponent>(
			[trackedSpatialEntities, cell = &cell](const Entity entity, const SpatialTrackComponent* spatialTrack) {
				auto it = trackedSpatialEntities->getEntitiesRef().find(spatialTrack->getId());
				if (it != trackedSpatialEntities->getEntitiesRef().end())
				{
					it->second.entity = entity;
					it->second.cell = cell->getPos();
				}
				else
				{
					ReportError("No tracked spatial entity record found for entity %d", spatialTrack->getId());
				}
			}
		);
	}
}

void World::fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	DeserializeEntityManager(mEntityManager, json.at("entity_manager"), jsonSerializerHolder);
	DeserializeComponentSetHolder(mWorldComponents, json.at("world_components"), jsonSerializerHolder);
	mSpatialData.fromJson(json.at("spatial_data"), jsonSerializerHolder);

	InitSpatialTrackedEntities(mSpatialData, mWorldComponents);
}

std::optional<std::pair<EntityView, CellPos>> World::getTrackedSpatialEntity(const StringId entityStringId)
{
	std::optional<std::pair<EntityView, CellPos>> result;

	if (const auto [trackedSpatialEntities] = getWorldComponents().getComponents<TrackedSpatialEntitiesComponent>(); trackedSpatialEntities)
	{
		const auto it = trackedSpatialEntities->getEntities().find(entityStringId);
		if (it != trackedSpatialEntities->getEntities().end())
		{
			if (WorldCell* cell = getSpatialData().getCell(it->second.cell))
			{
				result.emplace(EntityView(it->second.entity.getEntity(), cell->getEntityManager()), cell->getPos());
			}
		}
	}

	return result;
}

EntityView World::createTrackedSpatialEntity(StringId entityStringId, const CellPos pos)
{
	EntityView result = createSpatialEntity(pos);
	TrackedSpatialEntitiesComponent* trackedSpatialEntities = getWorldComponents().getOrAddComponent<TrackedSpatialEntitiesComponent>();

	trackedSpatialEntities->getEntitiesRef().insert_or_assign(entityStringId, SpatialEntity(result.getEntity(), pos));
	SpatialTrackComponent* trackComponent = result.addComponent<SpatialTrackComponent>();
	trackComponent->setId(entityStringId);
	return result;
}

EntityView World::createSpatialEntity(const CellPos pos)
{
	WorldCell& cell = getSpatialData().getOrCreateCell(pos);
	return EntityView(cell.getEntityManager().addEntity(), cell.getEntityManager());
}

void World::clearCaches()
{
	mEntityManager.clearCaches();
	mSpatialData.clearCaches();
}
