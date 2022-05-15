#pragma once

#include <vector>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Geometry/IntVector2D.h"

struct NavMesh
{
	NavMesh() = default;
	// NavMesh is a very heavy class, copying it does not have much sense
	NavMesh(const NavMesh&) = delete;
	NavMesh& operator=(const NavMesh&) = delete;

	struct Geometry
	{
		std::vector<Vector2D> vertices;
		std::vector<size_t> indexes;
		size_t verticesPerPoly = 3;
		size_t polygonsCount = 0;
		Vector2D navMeshStart{ZERO_VECTOR};
		Vector2D navMeshSize{ZERO_VECTOR};
		bool isCalculated = false;
	};

	struct InnerLinks
	{
		struct LinkData
		{
			LinkData() = default;
			LinkData(size_t neighbor, size_t borderPoint1, size_t borderPoint2)
				: neighbor(neighbor)
				, borderPoint1(borderPoint1)
				, borderPoint2(borderPoint2)
			{}

			size_t neighbor;
			size_t borderPoint1;
			size_t borderPoint2;
		};

		std::vector<std::vector<LinkData>> links;
		bool isCalculated = false;
	};

	struct SpatialHash
	{
		float cellSize = 100.0f;
		IntVector2D hashSize;
		std::vector<std::vector<size_t>> polygonsHash;
		bool isCalculated = false;
	};

	Geometry geometry;
	InnerLinks links;
	SpatialHash spatialHash;
};
