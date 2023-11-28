#include "worldsaveutils.h"

#include <algorithm>
#include <ranges>

#include "GameData/World.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/PathBlockingGeometryComponent.generated.h"

#include "Utils/World/GameDataLoader.h"
#include "Utils/Geometry/LightBlockingGeometry.h"
#include "Utils/AI/PathBlockingGeometry.h"

namespace Utils
{
	template<typename Component>
	void RemoveCellComponent(World& world)
	{
		TupleVector<Entity, Component*> components;
		for (std::pair<const CellPos, WorldCell>& cell : world.getSpatialData().getAllCells())
		{
			cell.second.getCellComponents().removeComponent(Component::GetTypeId());
		}
	}

	static void RefreshLightBlockingGeometry(World& world)
	{
		RemoveCellComponent<LightBlockingGeometryComponent>(world);

		TupleVector<WorldCell*, const CollisionComponent*, const TransformComponent*> components;
		world.getSpatialData().getAllCellManagers().getSpatialComponents<const CollisionComponent, const TransformComponent>(components);
		std::unordered_map<CellPos, std::vector<SimpleBorder>> lightBlockingGeometryPieces;
		LightBlockingGeometry::CalculateLightGeometry(lightBlockingGeometryPieces, components);

		for (auto& [cellPos, borders] : lightBlockingGeometryPieces)
		{
			// try to stabilize borders between saves
			std::ranges::sort(borders,
				[](const SimpleBorder& first, const SimpleBorder& second)
				{
					return first.a.x < second.a.x
						|| (first.a.x == second.a.x &&
							(first.b.x < second.b.x
							|| (first.b.x == second.b.x &&
								(first.a.y < second.a.y
								|| (first.a.y == second.a.y &&
									first.b.y < second.b.y)))));
				}
			);

			WorldCell& cell = world.getSpatialData().getOrCreateCell(cellPos);
			ComponentSetHolder& cellComponents = cell.getCellComponents();
			LightBlockingGeometryComponent* lightBlockingGeometry = cellComponents.getOrAddComponent<LightBlockingGeometryComponent>();
			lightBlockingGeometry->setBorders(std::move(borders));
		}
	}

	static void RefreshPathBlockingGeometry(World& world)
	{
		TupleVector<const CollisionComponent*, const TransformComponent*> components;
		world.getSpatialData().getAllCellManagers().getComponents<const CollisionComponent, const TransformComponent>(components);

		PathBlockingGeometryComponent* pathBlockingGeometry = world.getWorldComponents().getOrAddComponent<PathBlockingGeometryComponent>();

		PathBlockingGeometry::CalculatePathBlockingGeometry(pathBlockingGeometry->getPolygonsRef(), components);
	}

	void SaveWorld(World& world, const std::string& fileName, const Json::ComponentSerializationHolder& jsonSerializerHolder)
	{
		RefreshLightBlockingGeometry(world);
		RefreshPathBlockingGeometry(world);
		world.clearCaches();
		GameDataLoader::SaveWorld(world, ".", fileName, jsonSerializerHolder);
	}
}
