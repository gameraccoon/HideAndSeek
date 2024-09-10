#include "addentitygroupcommand.h"

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Serialization/Json/EntityManager.h"
#include "GameData/World.h"

AddEntityGroupCommand::AddEntityGroupCommand(const std::vector<nlohmann::json>& entities, const Json::ComponentSerializationHolder& jsonSerializerHolder, const Vector2D& shift)
	: EditorCommand(EffectBitset(EffectType::Entities))
	, mEntities(entities)
	, mSerializationHolder(jsonSerializerHolder)
	, mShift(shift)
{
}

void AddEntityGroupCommand::doCommand(CommandExecutionContext& context)
{
	auto addEntity = [](EntityManager& entityManager, size_t editorId, const nlohmann::json& serializedObject, const Json::ComponentSerializationHolder& serializationHolder) {
		const Entity entity = Json::CreatePrefabInstance(entityManager, serializedObject, serializationHolder);
		Utils::SetEntityId(entity, editorId, entityManager);
		return entity;
	};

	if (mCreatedEntities.empty())
	{
		mCreatedEntities.reserve(mEntities.size());
		mCreatedEntityPositions.reserve(mEntities.size());

		EntityManager& temporaryEntityManager = context.world->getEntityManager();
		for (const auto& serializedObject : mEntities)
		{
			const size_t editorId = context.getEditorIdGenerator().getNextId();
			const Entity entity = addEntity(temporaryEntityManager, editorId, serializedObject, mSerializationHolder);

			// move entity to the correct cell
			auto [transform] = temporaryEntityManager.getEntityComponents<TransformComponent>(entity);
			if (!transform)
			{
				transform = temporaryEntityManager.addComponent<TransformComponent>(entity);
			}

			Vector2D newPos = transform->getLocation() + mShift;
			transform->setLocation(newPos);

			const CellPos cellPos = SpatialWorldData::GetCellForPos(newPos);
			WorldCell& cell = context.world->getSpatialData().getOrCreateCell(cellPos);

			temporaryEntityManager.transferEntityTo(cell.getEntityManager(), entity);
			mCreatedEntities.emplace_back(editorId, cellPos);
			mCreatedEntityPositions.emplace_back(newPos);
		}
	}
	else
	{
		for (size_t i = 0, iSize = mCreatedEntities.size(); i < iSize; ++i)
		{
			if (!mCreatedEntities[i].cellPos.has_value())
			{
				std::cout << "AddEntityGroupCommand::doCommand: Entity " << mCreatedEntities[i].editorUniqueId << " has no cellPos\n";
				continue;
			}

			WorldCell& cell = context.world->getSpatialData().getOrCreateCell(*mCreatedEntities[i].cellPos);
			EntityManager& cellEntityManager = cell.getEntityManager();
			const Entity entity = addEntity(cellEntityManager, mCreatedEntities[i].editorUniqueId, mEntities[i], mSerializationHolder);

			auto [transform] = cellEntityManager.getEntityComponents<TransformComponent>(entity);
			if (!transform)
			{
				transform = cellEntityManager.addComponent<TransformComponent>(entity);
			}
			transform->setLocation(mCreatedEntityPositions[i]);
		}
	}
}

void AddEntityGroupCommand::undoCommand(CommandExecutionContext& context)
{
	for (auto [editorId, cellPos] : mCreatedEntities)
	{
		if (!cellPos.has_value())
		{
			std::cout << "AddEntityGroupCommand::undoCommand: Entity " << editorId << " has no cellPos\n";
			continue;
		}

		WorldCell& cell = context.world->getSpatialData().getOrCreateCell(*cellPos);
		const OptionalEntity entity = Utils::GetEntityFromId(editorId, cell.getEntityManager());
		if (!entity.isValid())
		{
			std::cout << "AddEntityGroupCommand::undoCommand: Could not find entity with id " << editorId << "\n";
			continue;
		}
		cell.getEntityManager().removeEntity(entity.getEntity());
	}
}
