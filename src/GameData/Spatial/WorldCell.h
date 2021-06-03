#pragma once

#include <nlohmann/json_fwd.hpp>

#include "GameData/EcsDefinitions.h"

#include "GameData/Core/Vector2D.h"
#include "GameData/Spatial/CellPos.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

class WorldCell
{
public:
	WorldCell(const CellPos& pos, const ComponentFactory& componentFactory, RaccoonEcs::EntityGenerator& entityGenerator);

	AsyncEntityManager& getEntityManager() { return mAsycnEntityManager; }
	[[nodiscard]] const AsyncEntityManager& getEntityManager() const { return mAsycnEntityManager; }
	ComponentSetHolder& getCellComponents() { return mCellComponents; }
	[[nodiscard]] const ComponentSetHolder& getCellComponents() const { return mCellComponents; }

	[[nodiscard]] CellPos getPos() const { return mPos; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	void clearCaches();
	[[nodiscard]] bool hasAnyData() const;

private:
	EntityManager mEntityManager;
	AsyncEntityManager mAsycnEntityManager{mEntityManager};
	ComponentSetHolder mCellComponents;
	CellPos mPos;
};
