#include "Base/precomp.h"

#include "GameData/World.h"

#include <nlohmann/json.hpp>

#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/SpatialTrackComponent.generated.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"
#include "GameData/Serialization/Json/EntityManager.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

World::World(const ComponentFactory& componentFactory, RaccoonEcs::EntityGenerator& entityGenerator)
	: mEntityManager(componentFactory, entityGenerator)
	, mWorldComponents(componentFactory)
	, mSpatialData(componentFactory, entityGenerator)
{
}

nlohmann::json World::toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	return nlohmann::json{
		{"entity_manager", Json::SerializeEntityManager(mEntityManager, jsonSerializerHolder)},
		{"world_components", Json::SerializeComponentSetHolder(mWorldComponents, jsonSerializerHolder)},
		{"spatial_data", mSpatialData.toJson(jsonSerializerHolder)}
	};
}

static void InitSpatialTrackedEntities(SpatialWorldData& spatialData, ComponentSetHolder& worldComponents)
{
	auto [trackedSpatialEntities] = worldComponents.getComponents<TrackedSpatialEntitiesComponent>();

	auto& cells = spatialData.getAllCells();
	for (auto& cellPair : cells)
	{
		cellPair.second.getEntityManager().forEachComponentSet<const SpatialTrackComponent>(
			[trackedSpatialEntities, cell = &cellPair.second](const SpatialTrackComponent* spatialTrack)
		{
			auto it = trackedSpatialEntities->getEntitiesRef().find(spatialTrack->getId());
			if (it != trackedSpatialEntities->getEntitiesRef().end())
			{
				it->second.cell = cell->getPos();
			}
			else
			{
				ReportError("No tracked spatial entity record found for entity %d", spatialTrack->getId());
			}
		});
	}
}

void World::fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	Json::DeserializeEntityManager(mEntityManager, json.at("entity_manager"), jsonSerializerHolder);
	Json::DeserializeComponentSetHolder(mWorldComponents, json.at("world_components"), jsonSerializerHolder);
	mSpatialData.fromJson(json.at("spatial_data"), jsonSerializerHolder);

	InitSpatialTrackedEntities(mSpatialData, mWorldComponents);
}

std::optional<std::pair<EntityView, CellPos>> World::getTrackedSpatialEntity(StringId entityStringId)
{
	std::optional<std::pair<EntityView, CellPos>> result;
	const auto [trackedSpatialEntities] = getWorldComponents().getComponents<TrackedSpatialEntitiesComponent>();

	if (trackedSpatialEntities)
	{
		auto it = trackedSpatialEntities->getEntities().find(entityStringId);
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

EntityView World::createTrackedSpatialEntity(StringId entityStringId, CellPos pos)
{
	EntityView result = createSpatialEntity(pos);
	TrackedSpatialEntitiesComponent* trackedSpatialEntities = getWorldComponents().getOrAddComponent<TrackedSpatialEntitiesComponent>();

	trackedSpatialEntities->getEntitiesRef().insert_or_assign(entityStringId, SpatialEntity(result.getEntity(), pos));
	SpatialTrackComponent* trackComponent = result.addComponent<SpatialTrackComponent>();
	trackComponent->setId(entityStringId);
	return result;
}

EntityView World::createSpatialEntity(CellPos pos)
{
	WorldCell& cell = getSpatialData().getOrCreateCell(pos);
	return EntityView(cell.getEntityManager().addEntity(), cell.getEntityManager());
}

void World::clearCaches()
{
	mEntityManager.clearCaches();
	mSpatialData.clearCaches();
}
