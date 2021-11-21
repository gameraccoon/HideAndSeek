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

	World(const World&) = delete;
	World(World&&) = delete;
	World operator=(const World&) = delete;
	World operator=(World&&) = delete;
	~World() = default;

	[[nodiscard]] AsyncEntityManager& getEntityManager() { return mAsyncEntityManager; }
	[[nodiscard]] const AsyncEntityManager& getEntityManager() const { return mAsyncEntityManager; }

	[[nodiscard]] ComponentSetHolder& getWorldComponents() { return mWorldComponents; }
	[[nodiscard]] const ComponentSetHolder& getWorldComponents() const { return mWorldComponents; }

	[[nodiscard]] SpatialWorldData& getSpatialData() { return mSpatialData; }
	[[nodiscard]] const SpatialWorldData& getSpatialData() const { return mSpatialData; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void fromJson(const nlohmann::json& json, const RaccoonEcs::InnerDataAccessor& dataAccessor, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	std::optional<std::pair<AsyncEntityView, CellPos>> getTrackedSpatialEntity(TrackedListConstFilter& filter, StringId entityStringId);
	AsyncEntityView createTrackedSpatialEntity(TrackedListAdder& trackingAdder, TrackAdder& trackAdder, EntityAdder& entityAdder, StringId entityStringId, CellPos pos);

	AsyncEntityView createSpatialEntity(EntityAdder& entityAdder, CellPos pos);

	void clearCaches();

private:
	EntityManager mEntityManager;
	AsyncEntityManager mAsyncEntityManager{mEntityManager};
	ComponentSetHolder mWorldComponents;
	SpatialWorldData mSpatialData;
};
