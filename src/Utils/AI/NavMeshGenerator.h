#pragma once

#include "Base/Types/TemplateAliases.h"

#include "GameData/AI/NavMesh.h"

namespace NavMeshGenerator
{
	// Fast can be 10-20 times faster
	// Good makes pathfinding a bit faster
	enum class HashGenerationType
	{
		Good,
		Fast
	};

	void GenerateNavMeshGeometry(NavMesh::Geometry& outGeometry, const std::vector<std::vector<Vector2D>>& pathBlockingGeometry, Vector2D start, Vector2D size);
	void LinkNavMesh(NavMesh::InnerLinks& outLinks, const NavMesh::Geometry& geometry);
	void BuildSpatialHash(NavMesh::SpatialHash& outSpatialHash, const NavMesh::Geometry& geometry, HashGenerationType generationType);
}
