#pragma once

#include <variant>

#include "GameData/EcsDefinitions.h"

#include "componentreference.h"

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
			return std::get<0>((*entityManager)->getEntityComponents<T>(*source.entity));
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
