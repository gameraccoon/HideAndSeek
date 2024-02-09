#include "changeentitygrouplocationcommand.h"

#include "../editorutils/editoridutils.h"

#include <GameData/World.h>
#include <GameData/Components/TransformComponent.generated.h>

ChangeEntityGroupLocationCommand::ChangeEntityGroupLocationCommand(const std::vector<EditorEntityReference>& entities, Vector2D shift)
	: EditorCommand(EffectBitset(EffectType::ComponentAttributes, EffectType::EntityLocations))
	, mShift(shift)
{
	mEditorEntityReferences.resize(entities.size());
	mOriginalCellsOfEntities.resize(entities.size());

	for (size_t i = 0; i < entities.size(); ++i)
	{
		mEditorEntityReferences[i] = entities[i].editorUniqueId;
		mOriginalCellsOfEntities[i] = entities[i].cellPos.value_or(CellPos{0, 0});
	}
}

void ChangeEntityGroupLocationCommand::doCommand(CommandExecutionContext& context)
{
	auto applyShift = [this, &context](size_t i, TransformComponent* component, WorldCell* cell, Entity entity)
	{
		component->setLocation(mModifiedPositionsOfEntities[i]);
		if (mModifiedCellsOfEntities[i] != mOriginalCellsOfEntities[i])
		{
			WorldCell& newCell = context.world->getSpatialData().getOrCreateCell(mModifiedCellsOfEntities[i]);
			cell->getEntityManager().transferEntityTo(newCell.getEntityManager(), entity);
		}
	};

	// if we never executed this command before, fill the data
	if (mOriginalPositionsOfEntities.empty()) {
		mOriginalPositionsOfEntities.resize(mEditorEntityReferences.size(), ZERO_VECTOR);
		mModifiedCellsOfEntities.resize(mEditorEntityReferences.size(), CellPos{0, 0});
		mModifiedPositionsOfEntities.resize(mEditorEntityReferences.size(), ZERO_VECTOR);

		for (size_t i = 0; i < mEditorEntityReferences.size(); ++i)
		{
			if (WorldCell* cell = context.world->getSpatialData().getCell(mOriginalCellsOfEntities[i]))
			{
				const OptionalEntity entity = Utils::GetEntityFromId(mEditorEntityReferences[i], cell->getEntityManager());
				if (!entity.isValid())
				{
					std::cout << "ChangeEntityGroupLocationCommand::doCommand: entity with id " << mEditorEntityReferences[i] << " not found\n";
					continue;
				}

				auto [component] = cell->getEntityManager().getEntityComponents<TransformComponent>(entity.getEntity());
				if (component)
				{
					const Vector2D originalPos = component->getLocation();
					mOriginalPositionsOfEntities[i] = originalPos;
					const Vector2D newPos = originalPos + mShift;
					mModifiedPositionsOfEntities[i] = newPos;
					const CellPos newCellPos = SpatialWorldData::GetCellForPos(newPos);
					mModifiedCellsOfEntities[i] = newCellPos;
					mModifiedPositionsOfEntities[i] = newPos;

					applyShift(i, component, cell, entity.getEntity());
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < mEditorEntityReferences.size(); ++i)
		{
			if (WorldCell* cell = context.world->getSpatialData().getCell(mOriginalCellsOfEntities[i]))
			{
				const OptionalEntity entity = Utils::GetEntityFromId(mEditorEntityReferences[i], cell->getEntityManager());
				if (!entity.isValid())
				{
					std::cout << "ChangeEntityGroupLocationCommand::doCommand: entity with id " << mEditorEntityReferences[i] << " not found\n";
					continue;
				}

				auto [component] = cell->getEntityManager().getEntityComponents<TransformComponent>(entity.getEntity());
				if (component)
				{
					applyShift(i, component, cell, entity.getEntity());
				}
			}
		}
	}
}

void ChangeEntityGroupLocationCommand::undoCommand(CommandExecutionContext& context)
{
	for (size_t i = 0; i < mEditorEntityReferences.size(); ++i)
	{
		if (WorldCell* cell = context.world->getSpatialData().getCell(mModifiedCellsOfEntities[i]))
		{
			const OptionalEntity entity = Utils::GetEntityFromId(mEditorEntityReferences[i], cell->getEntityManager());
			if (!entity.isValid())
			{
				std::cout << "ChangeEntityGroupLocationCommand::undoCommand: entity with id " << mEditorEntityReferences[i] << " not found\n";
				continue;
			}

			auto [component] = cell->getEntityManager().getEntityComponents<TransformComponent>(entity.getEntity());
			if (component)
			{
				component->setLocation(mOriginalPositionsOfEntities[i]);
				if (mOriginalCellsOfEntities[i] != mModifiedCellsOfEntities[i])
				{
					WorldCell& originalCell = context.world->getSpatialData().getOrCreateCell(mOriginalCellsOfEntities[i]);
					cell->getEntityManager().transferEntityTo(originalCell.getEntityManager(), entity.getEntity());
				}
			}
		}
	}
}
