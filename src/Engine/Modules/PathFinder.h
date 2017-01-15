#pragma once

#include <list>
#include <set>

#include "../Core/World.h"
#include "../Core/Vector2D.h"
#include "../Structures/PathPoint.h"

struct CalculationPoint
{
	CalculationPoint(PathPoint* point, float g, float h, CalculationPoint* cameFrom);
	PathPoint* point;
	float f, g, h;
	CalculationPoint* cameFrom;
};

/**
 * Class allows find path for some man.
 */
class PathFinder
{
public:
	PathFinder(World* world);
	~PathFinder();
	
	/** Find and save path from startPoint to endPoint in owner world */
	bool createNewPath(Vector2D startPoint, Vector2D endPoint);
	/**
	 * Get location of a next point of this path.
	 * @return next point or DestinationPoint if no more points of this path.
	 */
	Vector2D getNextPoint();

private:
	/** Current path */
	std::list<PathPoint*> mPath;
	Vector2D mDestinationPoint;
	World* mOwnerWorld;
	std::set<CalculationPoint*> mClosedSet;
	void reconstructPath(CalculationPoint* end);
};
