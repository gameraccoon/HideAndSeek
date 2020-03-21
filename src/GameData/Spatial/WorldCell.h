#pragma once

#include "ECS/EntityManager.h"
#include "ECS/ComponentSetHolder.h"

#include "GameData/Core/Vector2D.h"
#include "GameData/Spatial/CellPos.h"

class WorldCell
{
public:
	explicit WorldCell(const CellPos& pos);

	EntityManager& getEntityManager() { return mEntityManager; }
	ComponentSetHolder& getCellComponents() { return mCellComponents; }

	CellPos getPos() const { return mPos; }

	nlohmann::json toJson(const ComponentFactory& componentFactory) const;
	void fromJson(const nlohmann::json& json, const ComponentFactory& componentFactory);

private:
	EntityManager mEntityManager;
	ComponentSetHolder mCellComponents;
	CellPos mPos;
};