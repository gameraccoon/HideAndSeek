#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/World.h"
#include "GameData/Core/Hull.h"

#include "Utils/Geometry/Collide.h"
#include "Utils/Functional/ScopeFinalizer.h"

#include "GameData/Components/CollisionComponent.generated.h"

static void prepareCollision(CollisionComponent& collision, const Hull& hull)
{
	collision.setGeometry(hull);
	Collide::UpdateBoundingBox(&collision);
}

static bool getCollisionResult(const Vector2D& location1, const Hull& hull1, const Vector2D& location2, const Hull& hull2, Vector2D& outResist)
{
	CollisionComponent collision1;
	CollisionComponent collision2;

	prepareCollision(collision1, hull1);
	prepareCollision(collision2, hull2);

	return Collide::DoCollide(&collision1, location1, &collision2, location2, outResist);
}

TEST(Collision, Circular)
{
	Hull hull1;
	hull1.type = HullType::Circular;
	hull1.setRadius(10.0f);

	Hull hull2;
	hull2.type = HullType::Circular;
	hull2.setRadius(50.0f);

	Vector2D resist = ZERO_VECTOR;
	EXPECT_TRUE(getCollisionResult(Vector2D(20.0f, 10.0f), hull1, Vector2D(-10.0f, 15.0f), hull2, resist));
	EXPECT_FALSE(getCollisionResult(Vector2D(50.0f, 10.0f), hull1, Vector2D(-10.0f, 15.0f), hull2, resist));
}

TEST(Collision, Angular)
{
	Hull hull1;
	hull1.type = HullType::Circular;
	hull1.setRadius(1.0f);

	Hull hull2;
	hull2.type = HullType::Angular;
	hull2.points.emplace_back(10.0f, 9.0f);
	hull2.points.emplace_back(2.0f, 12.0f);
	hull2.points.emplace_back(3.0f, 5.0f);
	hull2.generateBorders();

	Vector2D resist = ZERO_VECTOR;

	EXPECT_FALSE(getCollisionResult(Vector2D(6.0f, 8.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_FALSE(getCollisionResult(Vector2D(2.0f, 4.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(2.0f, 2.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-2.0f, 3.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-2.0f, 1.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_FALSE(getCollisionResult(Vector2D(4.0f, 3.0f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-4.5f, 4.2f), hull1, Vector2D(-6.0f, -8.0f), hull2, resist));

	// inverted arguments
	EXPECT_FALSE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(6.0f, 8.0f), hull1, resist));
	EXPECT_FALSE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(2.0f, 4.0f), hull1, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(2.0f, 2.0f), hull1, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(-2.0f, 3.0f), hull1, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(-2.0f, 1.0f), hull1, resist));
	EXPECT_FALSE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(4.0f, 3.0f), hull1, resist));
	EXPECT_TRUE(getCollisionResult(Vector2D(-6.0f, -8.0f), hull2, Vector2D(-4.5f, 4.2f), hull1, resist));
}

TEST(Collision, CornerCase1)
{
	Hull hull1;
	hull1.type = HullType::Circular;
	hull1.setRadius(32.0f);

	Hull hull2;
	hull2.type = HullType::Angular;
	hull2.points.emplace_back(60.0f, -60.0f);
	hull2.points.emplace_back(60.0f, 60.0f);
	hull2.points.emplace_back(-60.0f, 60.0f);
	hull2.points.emplace_back(-60.0f, -60.0f);
	hull2.generateBorders();

	Vector2D resist = ZERO_VECTOR;

	EXPECT_FALSE(getCollisionResult(Vector2D(-0.00001f, 1002.0f), hull1, Vector2D(-60.0f, -60.0f), hull2, resist));
}
