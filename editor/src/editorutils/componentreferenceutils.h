#pragma once

#include <variant>

#include "ECS/Component.h"
#include "ECS/ComponentSetHolder.h"
#include "ECS/EntityManager.h"

#include "componentreference.h"

class World;
class ComponentFactory;

namespace Utils
{
	BaseComponent* GetComponent(const ComponentReference& reference, World* world);
	std::vector<BaseComponent*> GetComponents(const ComponentSourceReference& source, World* world);
	void AddComponent(const ComponentSourceReference& source, BaseComponent* component, World* world);
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
