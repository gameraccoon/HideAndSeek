#include "editoridutils.h"

#include "src/editorutils/editoridgenerator.h"

#include "GameData/Components/EditorIdComponent.generated.h"

namespace Utils
{
	OptionalEntity GetEntityFromId(size_t id, EntityManager& entityManager)
	{
		OptionalEntity outEntity;

		// not the most efficient way to do this, but this will do for now
		entityManager.forEachComponentSetWithEntity<EditorIdComponent>([&outEntity, id](Entity entity, EditorIdComponent* editorIdComponent)
		{
			if (editorIdComponent->getId() == id)
			{
				outEntity = entity;
			}
		});
		return outEntity;
	}

	void SetEntityId(Entity entity, size_t id, EntityManager& entityManager)
	{
		if (entityManager.doesEntityHaveComponent<EditorIdComponent>(entity))
		{
			auto [editorIdComponent] = entityManager.getEntityComponents<EditorIdComponent>(entity);
			editorIdComponent->setId(id);
		}
		else
		{
			EditorIdComponent* editorIdComponent = entityManager.addComponent<EditorIdComponent>(entity);
			editorIdComponent->setId(id);
		}
	}

	size_t GetEditorIdFromEntity(Entity entity, EntityManager& entityManager)
	{
		if (auto [editorId] = entityManager.getEntityComponents<EditorIdComponent>(entity); editorId)
		{
			return editorId->getId();
		}
		std::cout << "Entity does not have an EditorIdComponent" << std::endl;
		assert(false);
		return 0;
	}

	size_t GetOrCreateEditorIdFromEntity(Entity entity, EntityManager& entityManager, EditorIdGenerator& editorIdGenerator)
	{
		if (auto [editorId] = entityManager.getEntityComponents<EditorIdComponent>(entity); editorId)
		{
			return editorId->getId();
		}

		size_t newId = editorIdGenerator.getNextId();
		EditorIdComponent* editorIdComponent = entityManager.addComponent<EditorIdComponent>(entity);
		editorIdComponent->setId(newId);
		return newId;
	}
}
