#pragma once

#include "editorcommand.h"
#include "GameData/Spatial/SpatialEntity.h"

class World;

class AddSpatialEntityCommand : public EditorCommand
{
public:
	AddSpatialEntityCommand(const SpatialEntity& entity, const Vector2D& location);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	SpatialEntity mEntity;
	Vector2D mLocation;
};
