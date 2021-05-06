#pragma once

#include "editorcommand.h"

#include "GameData/Serialization/Json/JsonComponentSerializer.h"
#include "GameData/Spatial/SpatialEntity.h"

#include <nlohmann/json.hpp>

class World;

class RemoveEntitiesCommand : public EditorCommand
{
public:
	RemoveEntitiesCommand(const std::vector<SpatialEntity>& entities, const Json::ComponentSerializationHolder& serializerHolder);

	void doCommand(World* world) override;
	void undoCommand(World* world) override;

private:
	std::vector<SpatialEntity> mEntities;
	std::vector<nlohmann::json> mSerializedComponents;
	const Json::ComponentSerializationHolder& mComponentSerializerHolder;
};
