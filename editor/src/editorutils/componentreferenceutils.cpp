#include "componentreferenceutils.h"

#include "ECS/ComponentFactory.h"

#include "GameData/World.h"

namespace Utils
{
	BaseComponent* GetComponent(const ComponentReference& reference, World* world)
	{
		std::vector<BaseComponent*> components = GetComponents(reference.source, world);
		for (BaseComponent* component : components)
		{
			if (component->getComponentTypeName() == reference.componentTypeName)
			{
				return component;
			}
		}
		return nullptr;
	}

	std::vector<BaseComponent*> GetComponents(const ComponentSourceReference& source, World* world)
	{
		if (world)
		{
			auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				return (*entityManager)->getAllEntityComponents(*source.entity);
			}
			else if (auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				return (*componentHolder)->getAllComponents();
			}
		}
		return std::vector<BaseComponent*>();
	}

	void AddComponent(const ComponentSourceReference& source, BaseComponent* component, World* world, ComponentFactory& componentFactory)
	{
		if (world)
		{
			auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				(*entityManager)->addComponent(
					*source.entity,
					component,
					componentFactory.getTypeIDFromString(component->getComponentTypeName()).value()
				);
			}
			else if (auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				(*componentHolder)->addComponent(
					component,
					componentFactory.getTypeIDFromString(component->getComponentTypeName()).value()
				);
			}
		}
	}

	void RemoveComponent(const ComponentSourceReference& source, StringID componentTypeName, World* world, ComponentFactory& componentFactory)
	{
		if (world)
		{
			auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				(*entityManager)->removeComponent(
					*source.entity,
					componentFactory.getTypeIDFromString(componentTypeName).value()
				);
			}
			else if (auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				(*componentHolder)->removeComponent(
					componentFactory.getTypeIDFromString(componentTypeName).value()
				);
			}
		}
	}

	std::variant<ComponentSetHolder*, EntityManager*, std::nullptr_t>  GetBoundComponentHolderOrEntityManager(const ComponentSourceReference& source, World* world)
	{
		if (source.isWorld)
		{
			if (source.cellPos.has_value())
			{
				if (source.entity.has_value()) // spatial entity
				{
					if (WorldCell* cell = world->getSpatialData().getCell(*source.cellPos))
					{
						return &cell->getEntityManager();
					}
					else
					{
						return nullptr;
					}
				}
				else // cell component
				{
					if (WorldCell* cell = world->getSpatialData().getCell(*source.cellPos))
					{
						return &cell->getCellComponents();
					}
					else
					{
						return nullptr;
					}
				}
			}
			else if (source.entity.has_value()) // world entity
			{
				return &world->getEntityManager();
			}
			else // world component
			{
				return &world->getWorldComponents();
			}
		}
		else // game component
		{
			ReportFatalError("Game Components references are not supported yet");
			return nullptr;
		}
	}

}