#pragma once

#include <raccoon-ecs/async_operations.h>

#include <optional>

#include "GameData/EcsDefinitions.h"

#include "GameData/Spatial/SpatialWorldData.h"

class World
{
public:
	using TrackedListConstFilter = const RaccoonEcs::ComponentFilter<const class TrackedSpatialEntitiesComponent>;
	using TrackedListAdder = const RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>;
	using TrackAdder = const RaccoonEcs::ComponentAdder<class SpatialTrackComponent>;
	using EntityAdder = const RaccoonEcs::EntityAdder;

public:
	World(const ComponentFactory& componentFactory, RaccoonEcs::EntityGenerator& entityGenerator);

	[[nodiscard]] EntityManager& getEntityManager() { return mEntityManager; }
	[[nodiscard]] const EntityManager& getEntityManager() const { return mEntityManager; }

	[[nodiscard]] ComponentSetHolder& getWorldComponents() { return mWorldComponents; }
	[[nodiscard]] const ComponentSetHolder& getWorldComponents() const { return mWorldComponents; }

	[[nodiscard]] SpatialWorldData& getSpatialData() { return mSpatialData; }
	[[nodiscard]] const SpatialWorldData& getSpatialData() const { return mSpatialData; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	std::optional<std::pair<EntityView, CellPos>> getTrackedSpatialEntity(TrackedListConstFilter& filter, StringId entityStringId);
	EntityView createTrackedSpatialEntity(TrackedListAdder& trackingAdder, TrackAdder& trackAdder, EntityAdder& entityAdder, StringId entityStringId, CellPos pos);

	EntityView createSpatialEntity(EntityAdder& entityAdder, CellPos pos);

	void clearCaches();

private:
	EntityManager mEntityManager;
	ComponentSetHolder mWorldComponents;
	SpatialWorldData mSpatialData;
};
