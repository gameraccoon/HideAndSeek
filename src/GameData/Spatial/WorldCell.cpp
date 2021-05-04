#include "Base/precomp.h"

#include "GameData/Spatial/WorldCell.h"
#include "GameData/Serialization/Json/ComponentSetHolder.h"

#include <nlohmann/json.hpp>

WorldCell::WorldCell(const CellPos& pos, const ComponentFactory& componentFactory)
	: mEntityManager(componentFactory)
	, mCellComponents(componentFactory)
	, mPos(pos)
{
}

nlohmann::json WorldCell::toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const
{
	return nlohmann::json{
		{"entity_manager", mEntityManager.toJson(componentSerializers)},
		{"cell_components", Json::SerializeComponentSetHolder(mCellComponents, componentSerializers)},
	};
}

void WorldCell::fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers)
{
	mEntityManager.fromJson(json.at("entity_manager"), componentSerializers);

	Json::DeserializeComponentSetHolder(mCellComponents, json.at("cell_components"), componentSerializers);
}

void WorldCell::packForJsonSaving()
{
	mEntityManager.stableSortEntitiesById();
}

void WorldCell::clearCaches()
{
	mEntityManager.clearCaches();
}

bool WorldCell::hasAnyData() const
{
	return mEntityManager.hasAnyEntities() || mCellComponents.hasAnyComponents();
}
