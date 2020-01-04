#pragma once

#include <optional>

#include <Base/String/StringID.h>

#include <ECS/Entity.h>

#include <GameData/Spatial/CellPos.h>

/**
 * @brief A component can be stored in variety of ways and reference combinations
 * | Referenced by | In ComponentHolder (Unique) | In EntityManager (referenced by Entity) |
 * | (Unique)      | Game Components             |                                         |
 * | WorldID?      | World Components            | World Entities                          |
 * | CellPos       | Cell Components             | Spatial Entities                        |
 *
 * For example if a component is bound to a Spatial Entity, then it needs WorldID,
 * CellPos and Entity to be referenced properly.
 * In contrary to reference a Game Component nothing is needed (there's only one
 * such component possible).
 */
struct ComponentSourceReference
{
	/** nullopt indicates that the component stored not by EntityManager */
	std::optional<Entity> entity;
	/** nullopt indicates that the component is not bound to a cell */
	std::optional<CellPos> cellPos;
	/** false indicates that the component is not bound to any world */
	bool isWorld = false; // std::optional<WorldID>?
};

struct ComponentReference
{
	ComponentSourceReference source;
	StringID componentTypeName;
};
