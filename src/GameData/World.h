#pragma once

#include <optional>

#include "GameData/EcsDefinitions.h"

#include "GameData/Spatial/SpatialWorldData.h"

class World
{
public:
	World(const ComponentFactory& componentFactory);

	[[nodiscard]] EntityManager& getEntityManager() { return mEntityManager; }
	[[nodiscard]] const EntityManager& getEntityManager() const { return mEntityManager; }

	[[nodiscard]] ComponentSetHolder& getWorldComponents() { return mWorldComponents; }
	[[nodiscard]] const ComponentSetHolder& getWorldComponents() const { return mWorldComponents; }

	[[nodiscard]] SpatialWorldData& getSpatialData() { return mSpatialData; }
	[[nodiscard]] const SpatialWorldData& getSpatialData() const { return mSpatialData; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	std::optional<std::pair<EntityView, CellPos>> getTrackedSpatialEntity(StringId entityStringId);
	EntityView createTrackedSpatialEntity(StringId entityStringId, CellPos pos);
	EntityView createSpatialEntity(CellPos pos);

	void clearCaches();

private:
	EntityManager mEntityManager;
	ComponentSetHolder mWorldComponents;
	SpatialWorldData mSpatialData;
};
