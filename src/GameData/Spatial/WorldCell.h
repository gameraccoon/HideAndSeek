#pragma once

#include "GameData/EcsDefinitions.h"

#include "GameData/Core/Vector2D.h"
#include "GameData/Spatial/CellPos.h"

class WorldCell
{
public:
	WorldCell(const CellPos& pos, const ComponentFactory& componentFactory);

	EntityManager& getEntityManager() { return mEntityManager; }
	[[nodiscard]] const EntityManager& getEntityManager() const { return mEntityManager; }
	ComponentSetHolder& getCellComponents() { return mCellComponents; }
	[[nodiscard]] const ComponentSetHolder& getCellComponents() const { return mCellComponents; }

	[[nodiscard]] CellPos getPos() const { return mPos; }

	[[nodiscard]] nlohmann::json toJson(const Ecs::ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const Ecs::ComponentSerializersHolder& componentSerializers);

	void packForJsonSaving();
	void clearCaches();
	[[nodiscard]] bool hasAnyData() const;

private:
	EntityManager mEntityManager;
	ComponentSetHolder mCellComponents;
	CellPos mPos;
};
