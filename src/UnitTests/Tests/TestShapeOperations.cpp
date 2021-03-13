#include "Base/precomp.h"

#include <algorithm>
#include <random>

#include <gtest/gtest.h>

#include "Utils/Geometry/Collide.h"
#include "Utils/Geometry/ShapeOperations.h"

static std::vector<SimpleBorder> GenerateShape(const std::vector<Vector2D>& points)
{
	std::vector<SimpleBorder> result;
	size_t pointsCount = points.size();
	result.reserve(pointsCount);
	FOR_EACH_BORDER(pointsCount,
	{
		result.emplace_back(points[i], points[j]);
	});

	// borders can be in any order theoretically, so let's shuffle them
	// we need to use a predefined seed to have stable results (more or less)
	std::shuffle(std::begin(result), std::end(result), std::mt19937(42));

	return result;
}

static bool AreShapesEqual(const std::vector<SimpleBorder>& a, const std::vector<SimpleBorder>& b)
{
	if (a.size() != b.size())
	{
		return false;
	}

	auto a_copy = a;

	for (const SimpleBorder& border : b)
	{
		a_copy.erase(
			std::remove_if(
				a_copy.begin(),
				a_copy.end(),
				[&border](const SimpleBorder& borderB)
				{
					return border.a.isNearlyEqualTo(borderB.a) && border.b.isNearlyEqualTo(borderB.b);
				}),
			a_copy.end()
		);
	}
	return a_copy.empty();
}

static const std::vector<SimpleBorder>& GetShape(const std::vector<SimpleBorder>& vec)
{
	return vec;
}

static std::vector<SimpleBorder> GetShape(const std::vector<Vector2D>& vec)
{
	return GenerateShape(vec);
}

template<typename A, typename B, typename C>
void TestShapesUnionResultIsCorrect(const std::vector<A>& shape1, const std::vector<B>& shape2, const std::vector<C>& expectedResult)
{
	const std::vector<SimpleBorder>& preparedShape1(GetShape(shape1));
	const std::vector<SimpleBorder>& preparedShape2(GetShape(shape2));
	const std::vector<SimpleBorder>& expectedShape(GetShape(expectedResult));

	{
		std::vector<SimpleBorder> resultingShape = ShapeOperations::GetUnion(preparedShape1, preparedShape2);
		ShapeOperations::OptimizeShape(resultingShape);
		EXPECT_TRUE(AreShapesEqual(expectedShape, resultingShape));
	}
	{
		std::vector<SimpleBorder> resultingShape = ShapeOperations::GetUnion(preparedShape2, preparedShape1);
		ShapeOperations::OptimizeShape(resultingShape);
		EXPECT_TRUE(AreShapesEqual(expectedShape, resultingShape));
	}
}

template <typename T>
static std::vector<T> JoinVectors(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> result = a;
	result.reserve(a.size() + b.size());
	std::copy(b.begin(), b.end(), std::back_inserter(result));
	return result;
}

static std::vector<Vector2D> GetMovedShape(const std::vector<Vector2D>& shape, Vector2D shift)
{
	std::vector<Vector2D> result;
	result.reserve(shape.size());
	for (Vector2D originalPos : shape)
	{
		result.push_back(originalPos + shift);
	}
	return result;
}

TEST(ShapeOperations, Union_TwoTriangles)
{
	std::vector<Vector2D> shape1{{0.0f, 100.0f}, {100.0f, 0.0f}, {100.0f, 100.0f}};
	std::vector<Vector2D> shape2{{40.0f, 40.0f}, {80.0f, 70.0f}, {30.0f, 80.0f}};
	std::vector<Vector2D> expectedResult{{100.0f, 100.0f}, {0.0f, 100.0f}, {33.3333f, 66.6667f}, {40.0f, 40.0f}, {51.4286f, 48.5714f}, {100.0f, 0.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoAxisAlignedRects)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{60.0f, -10.0f}, {60.0f, 10.0f}, {-60.0f, 10.0f}, {-60.0f, -10.0f}};
	std::vector<Vector2D> expectedResult{{10.0f, -60.0f}, {10.0f, -10.0f}, {60.0f, -10.0f}, {60.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, 10.0f}, {-60.0f, 10.0f}, {-60.0f, -10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsWithOneSameBorder)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, -10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{10.0f, -10.0f}, {10.0f, 110.0f}, {-10.0f, 110.0f}, {-10.0f, -10.0f}};
	std::vector<Vector2D> expectedResult{{10.0f, -60.0f}, {10.0f, 110.0f}, {-10.0f, 110.0f}, {-10.0f, -60.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsWithOneOverlappingBorder)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, -10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{30.0f, -10.0f}, {30.0f, 110.0f}, {-30.0f, 110.0f}, {-30.0f, -10.0f}};
	std::vector<Vector2D> expectedResult{{30.0f, -10.0f}, {30.0f, 110.0f}, {-30.0f, 110.0f}, {-30.0f, -10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}, {10.0f, -60.0f}, {10.0f, -10.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsWithOneOverlappingBorderOneDirection)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, 20.0f}, {-10.0f, 20.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{30.0f, -60.0f}, {30.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, -60.0f}};
	std::vector<Vector2D> expectedResult{{30.0f, -60.0f}, {30.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 20.0f}, {-10.0f, 20.0f}, {-10.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, -60.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsWithOneOverlappingBorderOneDirectionFullyInside)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, -10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{30.0f, -60.0f}, {30.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, -60.0f}};
	std::vector<Vector2D> expectedResult(shape2);

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsWithTwoOverlappingBorders)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{10.0f, -10.0f}, {10.0f, 110.0f}, {-10.0f, 110.0f}, {-10.0f, -10.0f}};
	std::vector<Vector2D> expectedResult{{10.0f, -60.0f}, {10.0f, 110.0f}, {-10.0f, 110.0f}, {-10.0f, -60.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsIntersectionOnCorner)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{-30.0f, 10.0f}, {10.0f, -30.0f}, {20.0f, -20.0f}, {-20.0f, 20.0f}};
	std::vector<Vector2D> expectedResult{{10.0f, -60.0f}, {10.0f, -30.0f}, {20.0f, -20.0f}, {10.0f, -10.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, 10.0f}, {-20.0f, 20.0f}, {-30.0f, 10.0f}, {-10.0f, -10.0f}, {-10.0f, -60.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_TwoRectsCornerTouchingBorder)
{
	std::vector<Vector2D> shape1{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}};
	std::vector<Vector2D> shape2{{10.0f, 10.0f}, {50.0f, -30.0f}, {60.0f, -20.0f}, {20.0f, 20.0f}};
	std::vector<SimpleBorder> expectedShape = JoinVectors(GenerateShape(shape1), GenerateShape(shape2));

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoRectsCornerTouchingBorderFullyInside)
{
	std::vector<Vector2D> shape1{{10.0f, -20.0f}, {10.0f, 20.0f}, {-10.0f, 20.0f}, {-10.0f, -20.0f}};
	std::vector<Vector2D> shape2{{20.0f, -30.0f}, {20.0f, 0.0f}, {10.0f, 10.0f}, {20.0f, 20.0f}, {20.0f, 30.0f}, {-20.0f, 30.0f}, {-20.0f, -30.0f}};
	std::vector<SimpleBorder> expectedShape = GenerateShape(shape2);

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoRectsTouchingCorner)
{
	std::vector<SimpleBorder> shape1(GenerateShape({{-30.0f, 10.0f}, {-10.0f, -10.0f}, {10.0f, 10.0f}, {-10.0f, 30.0f}}));
	std::vector<SimpleBorder> shape2(GenerateShape({{10.0f, 10.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}}));
	std::vector<SimpleBorder> expectedShape = JoinVectors(shape1, shape2);

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoComplexFiguresBorderTouchingCorderOfFourBorders)
{
	std::vector<SimpleBorder> shape1a(GenerateShape({{-30.0f, 10.0f}, {-10.0f, -10.0f}, {10.0f, 10.0f}, {-10.0f, 30.0f}}));
	std::vector<SimpleBorder> shape1b(GenerateShape({{10.0f, 10.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}}));
	std::vector<Vector2D> shape2{{10.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 30.0f}, {10.0f, 30.0f}};
	std::vector<SimpleBorder> expectedShape = JoinVectors(shape1a, GenerateShape({{10.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 0.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}, {20.0f, 20.0f}, {20.0f, 30.0f}, {10.0f, 30.0f}}));

	TestShapesUnionResultIsCorrect(JoinVectors(shape1a, shape1b), shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoComplexFiguresOneCornerAnd4OverlappingBorders)
{
	std::vector<Vector2D> originalShape({{-20.0f, 0.0f}, {0.0f, -20.0f}, {20.0f, 0.0f}, {0.0f, 20.0f}});

	std::vector<SimpleBorder> shape1 = JoinVectors(GenerateShape(GetMovedShape(originalShape, {-20.0f, 0.0f})), GenerateShape(GetMovedShape(originalShape, {20.0f, 0.0f})));
	std::vector<SimpleBorder> shape2 = JoinVectors(GenerateShape(GetMovedShape(originalShape, {0.0f, -20.0f})), GenerateShape(GetMovedShape(originalShape, {0.0f, 20.0f})));

	std::vector<Vector2D> expectedShape({{-40.0f, 0.0f}, {0.0f, -40.0f}, {40.0f, 0.0f}, {0.0f, 40.0f}});

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoSameComplexFigures)
{
	std::vector<Vector2D> originalShape({{-20.0f, 0.0f}, {0.0f, -20.0f}, {20.0f, 0.0f}, {0.0f, 20.0f}});

	std::vector<SimpleBorder> shape1 = JoinVectors(GenerateShape(GetMovedShape(originalShape, {-20.0f, 0.0f})), GenerateShape(GetMovedShape(originalShape, {20.0f, 0.0f})));
	std::vector<SimpleBorder> shape2(shape1);

	std::vector<SimpleBorder> expectedShape(shape1);

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_NonConvexShape)
{
	std::vector<Vector2D> shape1{{-20.0f, 20.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 0.0f}, {-10.0f, 0.0f}, {-10.0f, 20.0f}};
	std::vector<Vector2D> shape2{{-30.0f, 5.0f}, {-30.0f, -5.0f}, {30.0f, -5.0f}, {30.0f, 5.0f}};
	std::vector<Vector2D> expectedResult{{-20.0f, 20.0f}, {-20.0f, 5.0f}, {-30.0f, 5.0f}, {-30.0f, -5.0f}, {-20.0f, -5.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, -5.0f}, {30.0f, -5.0f}, {30.0f, 5.0f}, {20.0f, 5.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 5.0f}, {-10.0f, 5.0f}, {-10.0f, 20.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_NonConvexShapeOneOverlappingBorder)
{
	std::vector<Vector2D> shape1{{-20.0f, 20.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 0.0f}, {-10.0f, 0.0f}, {-10.0f, 20.0f}};
	std::vector<Vector2D> shape2{{-30.0f, 10.0f}, {-30.0f, 0.0f}, {30.0f, 0.0f}, {30.0f, 10.0f}};
	std::vector<Vector2D> expectedResult{{-20.0f, 20.0f}, {-20.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, 0.0f}, {-20.0f, 0.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 0.0f}, {30.0f, 0.0f}, {30.0f, 10.0f}, {20.0f, 10.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 10.0f}, {-10.0f, 10.0f}, {-10.0f, 20.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, Union_NonConvexShapeOneOverlappingBorderOneDirection)
{
	std::vector<Vector2D> shape1{{-20.0f, 20.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 0.0f}, {-10.0f, 0.0f}, {-10.0f, 20.0f}};
	std::vector<Vector2D> shape2{{-30.0f, 0.0f}, {-30.0f, -10.0f}, {30.0f, -10.0f}, {30.0f, 0.0f}};
	std::vector<Vector2D> expectedResult{{-20.0f, 20.0f}, {-20.0f, 0.0f}, {-30.0f, 0.0f}, {-30.0f, -10.0f}, {-20.0f, -10.0f}, {-20.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, -10.0f}, {30.0f, -10.0f}, {30.0f, 0.0f}, {20.0f, 0.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}, {10.0f, 0.0f}, {-10.0f, 0.0f}, {-10.0f, 20.0f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, OptimizeShape_OneExtraPoint)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -40.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape));
}

TEST(ShapeOperations, OptimizeShape_TwoExtraPointOnABorder)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -40.0f}, {10.0f, 40.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape));
}

TEST(ShapeOperations, OptimizeShape_DuplicatedPoint)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape));
}

TEST(ShapeOperations, OptimizeShape_TwoFiguresTouchingWithMirroredAngle)
{
	std::vector<SimpleBorder> tempShape1 = GenerateShape(std::vector<Vector2D>{{0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}});
	std::vector<SimpleBorder> tempShape2 = GenerateShape(std::vector<Vector2D>{{10.0f, 10.0f}, {20.0f, 10.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}});
	std::vector<SimpleBorder> testShape = JoinVectors(std::move(tempShape1), std::move(tempShape2));
	// expect the shape to be unchanged, since we don't want borders with opposing directions to merge
	std::vector<SimpleBorder> expectedShape = testShape;
	ShapeOperations::OptimizeShape(testShape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, testShape));
}

TEST(ShapeOperations, OptimizeShape_BorderSplitTwice)
{
	std::vector<SimpleBorder> testShape{
		{{-30.0f, 10.0f}, {-30.0f, -60.0f}},
		{{30.0f, 10.0f}, {-30.0f, 10.0f}},
		{{30.0f, -60.0f}, {30.0f, 10.0f}},
		{{-10.0f, -60.0f}, {10.0f, -60.0f}},
		{{-30.0f, -60.0f}, {-10.0f, -60.0f}},
		{{10.0f, -60.0f}, {30.0f, -60.0f}}
	};

	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{30.0f, -60.0f}, {30.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, -60.0f}});
	ShapeOperations::OptimizeShape(testShape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, testShape));
}
