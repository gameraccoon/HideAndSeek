#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/Spatial/SpatialWorldData.h"

TEST(SpatialPartitioning, CellPostionTransform)
{
	CellPos cell(20, 2);
	Vector2D pos(SpatialWorldData::CellSize * 1.3f, SpatialWorldData::CellSize * -4.8f);
	SpatialWorldData::TransformCellPos(cell, pos);

	EXPECT_EQ(21, cell.x);
	EXPECT_EQ(-3, cell.y);
	EXPECT_NEAR(SpatialWorldData::CellSize * 0.3f, pos.x, 0.0001f);
	EXPECT_NEAR(SpatialWorldData::CellSize * 0.2f, pos.y, 0.0001f);

	pos = Vector2D (SpatialWorldData::CellSize * -4.4f, SpatialWorldData::CellSize * 1.1f);
	SpatialWorldData::TransformCellPos(cell, pos);

	EXPECT_EQ(16, cell.x);
	EXPECT_EQ(-2, cell.y);
	EXPECT_NEAR(SpatialWorldData::CellSize * 0.6f, pos.x, 0.0001f);
	EXPECT_NEAR(SpatialWorldData::CellSize * 0.1f, pos.y, 0.0001f);
}
