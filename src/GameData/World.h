#pragma once

#include <optional>

#include "ECS/EntityManager.h"
#include "ECS/ComponentSetHolder.h"
#include "ECS/EntityView.h"

#include "GameData/Spatial/SpatialWorldData.h"

struct ComponentSerializersHolder;

class World
{
public:
	[[nodiscard]] EntityManager& getEntityManager() { return mEntityManager; }
	[[nodiscard]] const EntityManager& getEntityManager() const { return mEntityManager; }

	[[nodiscard]] ComponentSetHolder& getWorldComponents() { return mWorldComponents; }
	[[nodiscard]] const ComponentSetHolder& getWorldComponents() const { return mWorldComponents; }

	[[nodiscard]] SpatialWorldData& getSpatialData() { return mSpatialData; }
	[[nodiscard]] const SpatialWorldData& getSpatialData() const { return mSpatialData; }

	[[nodiscard]] nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers);

	std::optional<std::pair<EntityView, CellPos>> getTrackedSpatialEntity(StringId entityStringId);
	EntityView createTrackedSpatialEntity(StringId entityStringId, CellPos pos);
	EntityView createSpatialEntity(CellPos pos);

	void packForJsonSaving();
	void clearCaches();

private:
	EntityManager mEntityManager;
	ComponentSetHolder mWorldComponents;
	SpatialWorldData mSpatialData;
};
