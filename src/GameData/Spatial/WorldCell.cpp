#include "Base/precomp.h"

#include "GameData/Spatial/WorldCell.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"
#include "GameData/Serialization/Json/EntityManager.h"

#include <nlohmann/json.hpp>

WorldCell::WorldCell(const CellPos& pos, const ComponentFactory& componentFactory)
	: mEntityManager(componentFactory)
	, mCellComponents(componentFactory)
	, mPos(pos)
{
}

nlohmann::json WorldCell::toJson(const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	return nlohmann::json{
		{"entity_manager", Json::SerializeEntityManager(mEntityManager, jsonSerializerHolder)},
		{"cell_components", Json::SerializeComponentSetHolder(mCellComponents, jsonSerializerHolder)},
	};
}

void WorldCell::fromJson(const nlohmann::json& json, const Json::ComponentSerializationHolder& jsonSerializerHolder)
{
	Json::DeserializeEntityManager(mEntityManager, json.at("entity_manager"), jsonSerializerHolder);
	Json::DeserializeComponentSetHolder(mCellComponents, json.at("cell_components"), jsonSerializerHolder);
}

void WorldCell::clearCaches()
{
	mEntityManager.clearCaches();
}

bool WorldCell::hasAnyData() const
{
	return mEntityManager.hasAnyEntities() || mCellComponents.hasAnyComponents();
}
