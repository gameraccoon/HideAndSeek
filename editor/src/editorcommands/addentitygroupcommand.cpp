#include "addentitygroupcommand.h"

#include <QtWidgets/qcombobox.h>

#include "GameData/World.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Serialization/Json/EntityManager.h"

#include "src/EditorDataAccessor.h"

AddEntityGroupCommand::AddEntityGroupCommand(const std::vector<nlohmann::json>& entities, const Json::ComponentSerializationHolder& jsonSerializerHolder, const Vector2D& shift)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntities(entities)
	, mSerializationHolder(jsonSerializerHolder)
	, mShift(shift)
{
}

void AddEntityGroupCommand::doCommand(World* world)
{
	RaccoonEcs::EntityTransferer entityTransferer(gEditorDataAccessor);
	mCreatedEntities.clear();
	CellPos initialPos(0, 0);
	WorldCell& cell = world->getSpatialData().getOrCreateCell(initialPos);
	EntityManager& cellEntityManager = gEditorDataAccessor.getSingleThreadedEntityManager(cell.getEntityManager());
	for (const auto& serializedObject : mEntities)
	{
		CellPos cellPos = initialPos;
		Entity entity = Json::CreatePrefabInstance(cellEntityManager, serializedObject, mSerializationHolder);
		auto [transform] = cellEntityManager.getEntityComponents<TransformComponent>(entity);
		if (transform)
		{
			Vector2D newPos = transform->getLocation() + mShift;
			transform->setLocation(newPos);

			cellPos = SpatialWorldData::GetCellForPos(newPos);
			if (cellPos != initialPos)
			{
				WorldCell& otherCell = world->getSpatialData().getOrCreateCell(cellPos);
				// the component pointer get invalidated from this line
				entityTransferer.transferEntity(cell.getEntityManager(), otherCell.getEntityManager(), entity);
			}
		}
		mCreatedEntities.emplace_back(entity, cellPos);
	}
}

void AddEntityGroupCommand::undoCommand(World* world)
{
	for (auto [entity, cellPos] : mCreatedEntities)
	{
		WorldCell& cell = world->getSpatialData().getOrCreateCell(cellPos);
		gEditorDataAccessor.getSingleThreadedEntityManager(cell.getEntityManager()).removeEntity(entity.getEntity());
	}
}
