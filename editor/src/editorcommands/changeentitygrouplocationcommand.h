#pragma once

#include "editorcommand.h"

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Spatial/SpatialEntity.h"

class World;

class ChangeEntityGroupLocationCommand : public EditorCommand
{
public:
	using SetterFunction = void (World::*)(const OptionalEntity&);

public:
	ChangeEntityGroupLocationCommand(const std::vector<SpatialEntity>& entities, Vector2D shift);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

	const std::vector<SpatialEntity>& getOriginalEntities() const { return mOriginalEntities; }
	const std::vector<SpatialEntity>& getModifiedEntities() const { return mModifiedEntities; }

private:
	const Vector2D mShift;
	const std::vector<SpatialEntity> mOriginalEntities;
	std::vector<Vector2D> mOriginalEntitiesPos;

	std::vector<SpatialEntity> mModifiedEntities;
	std::vector<Vector2D> mModifiedEntitiesPos;
};
