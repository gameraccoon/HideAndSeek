#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "GameUtils/Geometry/Collide.h"

TEST(CollisionUtils, AreLinesParallel)
{
	EXPECT_TRUE(Collide::AreLinesParallel({10.0f, 20.0f}, {-10.0f, 20.0f}, {5.0f, -3.0f}, {-5.0f, -3.0f}));
	EXPECT_FALSE(Collide::AreLinesParallel({10.0f, 20.0f}, {-10.0f, 20.0f}, {5.0f, -3.0f}, {-5.0f, 3.0f}));
	EXPECT_TRUE(Collide::AreLinesParallel({20.0f, 10.0f}, {20.0f, -10.0f}, {-3.0f, 5.0f}, {-3.0f, -5.0f}));
	EXPECT_FALSE(Collide::AreLinesParallel({20.0f, 10.0f}, {20.0f, -10.0f}, {-3.0f, 5.0f}, {3.0f, -5.0f}));
	EXPECT_TRUE(Collide::AreLinesParallel({-10.0f, -10.0f}, {10.0f, 20.0f}, {5.0f, 0.0f}, {25.0f, 30.0f}));
	EXPECT_TRUE(Collide::AreLinesParallel({-10.0f, -10.0f}, {10.0f, 20.0f}, {25.0f, 30.0f}, {5.0f, 0.0f}));
	EXPECT_FALSE(Collide::AreLinesParallel({-10.0f, -10.0f}, {10.0f, 20.0f}, {25.0f, -30.0f}, {5.0f, 0.0f}));
}

TEST(CollisionUtils, DistanceToLineSegmentSq_CloseToPointA)
{
	constexpr Vector2D point{10.0f, 10.0f};
	constexpr Vector2D lineA{15.0f, 15.0f};
	constexpr Vector2D lineB{18.0f, 21.0f};

	EXPECT_NEAR(50.0f, Collide::DistanceToLineSegmentSq(lineA, lineB, point), 0.001f);
}

TEST(CollisionUtils, DistanceToLineSegmentSq_CloseToPointB)
{
	constexpr Vector2D point{20.0f, 23.0f};
	constexpr Vector2D lineA{15.0f, 15.0f};
	constexpr Vector2D lineB{18.0f, 21.0f};

	EXPECT_NEAR(8.0f, Collide::DistanceToLineSegmentSq(lineA, lineB, point), 0.001f);
}

TEST(CollisionUtils, DistanceToLineSegmentSq_CloseToMiddle)
{
	constexpr Vector2D point{16.0f, 14.0f};
	constexpr Vector2D lineA{10.0f, 10.0f};
	constexpr Vector2D lineB{20.0f, 20.0f};

	EXPECT_NEAR(2.0f, Collide::DistanceToLineSegmentSq(lineA, lineB, point), 0.001f);
}

TEST(CollisionUtils, FindDistanceToConvexHullSq_CloseToPoint)
{
	const std::vector<Vector2D> hull{{15.0f, 10.0f}, {30.0f, -10.0f}, {32.0f, 20.0f}};
	constexpr Vector2D point{10.0f, 10.0f};

	EXPECT_NEAR(25.0f, Collide::FindDistanceToConvexHullSq(hull, point), 0.001f);
}

TEST(CollisionUtils, FindDistanceToConvexHullSq_CloseToBorder)
{
	const std::vector<Vector2D> hull{{15.0f, -10.0f}, {15.0f, 20.0f}, {40.0f, 10.0f}};
	constexpr Vector2D point{10.0f, 10.0f};

	EXPECT_NEAR(25.0f, Collide::FindDistanceToConvexHullSq(hull, point), 0.001f);
}

TEST(CollisionUtils, FindDistanceToConvexHullSq_CloseToBorderWithFarPoints)
{
	const std::vector<Vector2D> hull{{11.0f, -100.0f}, {11.0f, 110.0f}, {12.0f, 10.0f}};
	constexpr Vector2D point{10.0f, 10.0f};

	EXPECT_NEAR(1.0f, Collide::FindDistanceToConvexHullSq(hull, point), 0.001f);
}
