#pragma once

#include <iostream>
#include <variant>

#include "componentreference.h"
#include "editoridutils.h"

#include "GameData/EcsDefinitions.h"

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
		const auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
		if (const auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
		{
			const OptionalEntity entity = Utils::GetEntityFromId(*source.editorUniqueId, **entityManager);
			if (!entity.isValid())
			{
				std::cout << "Could not find entity with id " << *source.editorUniqueId << "\n";
				return nullptr;
			}
			return std::get<0>((*entityManager)->getEntityComponents<T>(entity.getEntity()));
		}
		if (const auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
		{
			return std::get<0>((*componentHolder)->getComponents<T>());
		}
		return nullptr;
	}
} // namespace Utils
