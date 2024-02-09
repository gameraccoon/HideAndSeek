#pragma once

#include "GameData/EcsDefinitions.h"

class EditorIdGenerator;

namespace Utils
{
	OptionalEntity GetEntityFromId(size_t id, EntityManager& entityManager);
	void SetEntityId(Entity entity, size_t id, EntityManager& entityManager);
	size_t GetEditorIdFromEntity(Entity entity, EntityManager& entityManager);
	size_t GetOrCreateEditorIdFromEntity(Entity entity, EntityManager& entityManager, EditorIdGenerator& editorIdGenerator);
}
