#include "addspatialentitycommand.h"

#include "../editorutils/editoridutils.h"

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

AddSpatialEntityCommand::AddSpatialEntityCommand(const CellPos cellPos, const Vector2D& location)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mCellPos(cellPos)
	, mLocation(location)
{
}

void AddSpatialEntityCommand::doCommand(CommandExecutionContext& context)
{
	if (!mEditorEntityId)
	{
		mEditorEntityId = context.getEditorIdGenerator().getNextId();
	}

	WorldCell& cell = context.world->getSpatialData().getOrCreateCell(mCellPos);
	EntityManager& cellEntityManager = cell.getEntityManager();

	const Entity entity = cellEntityManager.addEntity();

	Utils::SetEntityId(entity, *mEditorEntityId, cellEntityManager);

	TransformComponent* transform = cellEntityManager.addComponent<TransformComponent>(entity);
	transform->setLocation(mLocation);
}

void AddSpatialEntityCommand::undoCommand(CommandExecutionContext& context)
{
	if (WorldCell* cell = context.world->getSpatialData().getCell(mCellPos))
	{
		EntityManager& cellEntityManager = cell->getEntityManager();
		const OptionalEntity entity = Utils::GetEntityFromId(*mEditorEntityId, cellEntityManager);
		if (!entity.isValid())
		{
			std::cout << "AddSpatialEntityCommand::undoCommand: entity with id " << *mEditorEntityId << " not found\n";
			return;
		}

		cellEntityManager.removeEntity(entity.getEntity());
	}
}
