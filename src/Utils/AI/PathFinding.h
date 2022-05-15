#pragma once

#include "GameData/Geometry/Vector2D.h"

struct NavMesh;

namespace PathFinding
{
	bool IsPointInsideConvexHull(Vector2D point, const std::vector<Vector2D>& hull);
	void FindPath(std::vector<Vector2D>& outPath, const NavMesh& navMesh, Vector2D start, Vector2D finish);
}
