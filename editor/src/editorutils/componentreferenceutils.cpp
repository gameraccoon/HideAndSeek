#include "componentreferenceutils.h"

#include "EngineCommon/Debug/Assert.h"

#include "GameData/World.h"

namespace Utils
{
	void* GetComponent(const ComponentReference& reference, World* world)
	{
		std::vector<TypedComponent> components = GetComponents(reference.source, world);
		for (TypedComponent& componentData : components)
		{
			if (componentData.typeId == reference.componentTypeName)
			{
				return componentData.component;
			}
		}
		return nullptr;
	}

	std::vector<TypedComponent> GetComponents(const ComponentSourceReference& source, World* world)
	{
		if (world)
		{
			const auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (const auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				const OptionalEntity entity = Utils::GetEntityFromId(*source.editorUniqueId, **entityManager);
				if (!entity.isValid())
				{
					std::cout << "Could not find entity with id " << *source.editorUniqueId << "\n";
					return {};
				}

				std::vector<TypedComponent> componentDatas;
				(*entityManager)->getAllEntityComponents(entity.getEntity(), componentDatas);
				return componentDatas;
			}
			if (const auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				return (*componentHolder)->getAllComponents();
			}
		}
		return std::vector<TypedComponent>();
	}

	void AddComponent(const ComponentSourceReference& source, const TypedComponent componentData, World* world)
	{
		if (world)
		{
			const auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (const auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				const OptionalEntity entity = Utils::GetEntityFromId(*source.editorUniqueId, **entityManager);
				if (!entity.isValid())
				{
					std::cout << "Could not find entity with id " << *source.editorUniqueId << "\n";
					return;
				}
				(*entityManager)->addComponent(entity.getEntity(), componentData.component, componentData.typeId);
			}
			else if (const auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				(*componentHolder)->addComponent(componentData.component, componentData.typeId);
			}
		}
	}

	void RemoveComponent(const ComponentSourceReference& source, const StringId componentTypeName, World* world)
	{
		if (world)
		{
			const auto componentHolderOrEntityManager = GetBoundComponentHolderOrEntityManager(source, world);
			if (const auto entityManager = std::get_if<EntityManager*>(&componentHolderOrEntityManager))
			{
				const OptionalEntity entity = Utils::GetEntityFromId(*source.editorUniqueId, **entityManager);
				if (!entity.isValid())
				{
					std::cout << "Could not find entity with id " << *source.editorUniqueId << "\n";
					return;
				}
				(*entityManager)->removeComponent(entity.getEntity(), componentTypeName);
			}
			else if (const auto componentHolder = std::get_if<ComponentSetHolder*>(&componentHolderOrEntityManager))
			{
				(*componentHolder)->removeComponent(componentTypeName);
			}
		}
	}

	std::variant<ComponentSetHolder*, EntityManager*, std::nullptr_t> GetBoundComponentHolderOrEntityManager(const ComponentSourceReference& source, World* world)
	{
		if (source.isWorld)
		{
			if (source.cellPos.has_value())
			{
				if (source.editorUniqueId.has_value()) // spatial entity
				{
					if (WorldCell* cell = world->getSpatialData().getCell(*source.cellPos))
					{
						return &cell->getEntityManager();
					}
					return nullptr;
				}

				// cell component
				if (!source.cellPos.has_value())
				{
					std::cout << "Cell component source has no cellPos\n";
					return nullptr;
				}
				if (WorldCell* cell = world->getSpatialData().getCell(*source.cellPos))
				{
					return &cell->getCellComponents();
				}
				std::cout << "Could not find cell at " << source.cellPos->x << ", " << source.cellPos->y << "\n";
				return nullptr;
			}
			if (source.editorUniqueId.has_value()) // world entity
			{
				return &world->getEntityManager();
			}
			return &world->getWorldComponents();
		}

		// game component
		ReportFatalError("Game Components references are not supported yet");
		return nullptr;
	}

} // namespace Utils
