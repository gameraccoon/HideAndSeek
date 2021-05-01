#include "Base/precomp.h"

#include "GameData/World.h"

#include <nlohmann/json.hpp>

#include "ECS/Serialization/ComponentSerializersHolder.h"

#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/SpatialTrackComponent.generated.h"

World::World(const ComponentFactory& componentFactory)
	: mEntityManager(componentFactory)
	, mWorldComponents(componentFactory)
	, mSpatialData(componentFactory)
{
}

nlohmann::json World::toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const
{
	return nlohmann::json{
		{"entity_manager", mEntityManager.toJson(componentSerializers)},
		{"world_components", mWorldComponents.toJson(componentSerializers)},
		{"spatial_data", mSpatialData.toJson(componentSerializers)}
	};
}

static void InitSpatialTrackedEntities(SpatialWorldData& spatialData, ComponentSetHolder& worldComponents)
{
	auto [trackedSpatialEntities] = worldComponents.getComponents<TrackedSpatialEntitiesComponent>();

	spatialData.getAllCellManagers().forEachSpatialComponentSet<SpatialTrackComponent>([trackedSpatialEntities](WorldCell* cell, SpatialTrackComponent* spatialTrack)
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

void World::fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers)
{
	mEntityManager.fromJson(json.at("entity_manager"), componentSerializers);
	mWorldComponents.fromJson(json.at("world_components"), componentSerializers);
	mSpatialData.fromJson(json.at("spatial_data"), componentSerializers);

	InitSpatialTrackedEntities(mSpatialData, mWorldComponents);
}

std::optional<std::pair<EntityView, CellPos>> World::getTrackedSpatialEntity(StringId entityStringId)
{
	std::optional<std::pair<EntityView, CellPos>> result;
	auto [trackedSpatialEntities] = getWorldComponents().getComponents<TrackedSpatialEntitiesComponent>();

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
	auto result = createSpatialEntity(pos);
	auto [trackedSpatialEntities] = getWorldComponents().getComponents<TrackedSpatialEntitiesComponent>();
	if (trackedSpatialEntities == nullptr)
	{
		trackedSpatialEntities = getWorldComponents().addComponent<TrackedSpatialEntitiesComponent>();
	}
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

void World::packForJsonSaving()
{
	mEntityManager.stableSortEntitiesById();
	mSpatialData.packForJsonSaving();
}

void World::clearCaches()
{
	mEntityManager.clearCaches();
	mSpatialData.clearCaches();
}
