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

	EntityManager& getEntityManager() { return mEntityManager; }
	const EntityManager& getEntityManager() const { return mEntityManager; }
	ComponentSetHolder& getCellComponents() { return mCellComponents; }
	const ComponentSetHolder& getCellComponents() const { return mCellComponents; }

	CellPos getPos() const { return mPos; }

	[[nodiscard]] nlohmann::json toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder);
	void fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder);

	void clearCaches();
	bool hasAnyData() const;

private:
	EntityManager mEntityManager;
	ComponentSetHolder mCellComponents;
	CellPos mPos;
};
