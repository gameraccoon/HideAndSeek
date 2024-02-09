#pragma once

#include <variant>
#include <iostream>

#include "GameData/EcsDefinitions.h"

#include "componentreference.h"
#include "editoridutils.h"

class World;

namespace Utils
{
	void* GetComponent(const ComponentReference& reference, World* world);
	std::vector<TypedComponent> GetComponents(const ComponentSourceReference& source, World* world);
	void AddComponent(const ComponentSourceReference& source, TypedComponent componentData, World* world);
	void RemoveComponent(const ComponentSourceReference& source, StringId componentTypeName, World* world);

	std::variant<ComponentSetHolder*, EntityManager*, std::nullptr_t> GetBoundComponentHolderOrEntityManager(const ComponentSourceReference& source, World* world);

	template<typename T>
	T* GetComponent(const ComponentSourceReference& source, World* world)
	{
		auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
		if (auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
		{
			OptionalEntity entity = Utils::GetEntityFromId(*source.editorUniqueId, **entityManager);
			if (!entity.isValid())
			{
				std::cout << "Could not find entity with id " << *source.editorUniqueId << "\n";
				return nullptr;
			}
			return std::get<0>((*entityManager)->getEntityComponents<T>(entity.getEntity()));
		}
		else if (auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
		{
			return std::get<0>((*componentHolder)->getComponents<T>());
		}
		else
		{
			return nullptr;
		}
	}
}
