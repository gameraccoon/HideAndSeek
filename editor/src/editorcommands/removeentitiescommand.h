#pragma once

#include "editorcommand.h"

#include "ECS/Serialization/ComponentSerializersHolder.h"

#include "GameData/Spatial/SpatialEntity.h"

#include <nlohmann/json.hpp>

class World;

class RemoveEntitiesCommand : public EditorCommand
{
public:
	RemoveEntitiesCommand(const std::vector<SpatialEntity>& entities, const Ecs::ComponentSerializersHolder& serializerHolder);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	std::vector<SpatialEntity> mEntities;
	std::vector<nlohmann::json> mSerializedComponents;
	const Ecs::ComponentSerializersHolder& mComponentSerializerHolder;
};
