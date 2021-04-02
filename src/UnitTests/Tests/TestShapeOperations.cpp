#include "Base/precomp.h"

#include <algorithm>
#include <random>
#include <ranges>
#include <span>

#include <gtest/gtest.h>

#include "Base/Types/ComplexTypes/VectorUtils.h"

#include "Utils/Geometry/Collide.h"
#include "Utils/Geometry/ShapeOperations.h"

enum class ShapeOrder
{
	Ordered,
	Unordered
};

namespace VectorUtils {
	template <typename T, typename... Args>
	std::vector<T> JoinVectors(Args... inputVectors)
	{
		std::vector<T> result;
		size_t size = (inputVectors.size() + ...);
		result.reserve(size);
		(std::ranges::copy(inputVectors..., std::back_inserter(result)));
		return result;
	}
}

static std::vector<SimpleBorder> GenerateShape(const std::vector<Vector2D>& points, ShapeOrder order = ShapeOrder::Unordered)
{
	std::vector<SimpleBorder> result;
	size_t pointsCount = points.size();
	result.reserve(pointsCount);
	FOR_EACH_BORDER(pointsCount,
	{
		result.emplace_back(points[i], points[j]);
	});

	if (order == ShapeOrder::Unordered)
	{
		// borders can be in any order theoretically, so let's shuffle them
		// we need to use a predefined seed to have stable results (more or less)
		std::shuffle(std::begin(result), std::end(result), std::mt19937(42));
	}

	return result;
}

enum class ShapeEquality
{
	Ordered,
	Shuffled,
	Exact
};

static bool AreShapesEqual(const std::span<const SimpleBorder> a, const std::span<const SimpleBorder> b, const ShapeEquality equalityType)
{
	if (a.size() != b.size())
	{
		return false;
	}

	if (equalityType == ShapeEquality::Shuffled)
	{
		std::vector a_copy(a.begin(), a.end());

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
	else if (equalityType == ShapeEquality::Ordered)
	{
		std::vector a_copy(a.begin(), a.end());

		auto it = std::find(a_copy.begin(), a_copy.end(), b[0]);
		if (it == a_copy.end())
		{
			return false;
		}

		std::rotate(a_copy.begin(), it, a_copy.end());

		return std::equal(a_copy.begin(), a_copy.end(), b.begin());
	}
	else
	{
		return std::equal(a.begin(), a.end(), b.begin());
	}
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
		EXPECT_TRUE(AreShapesEqual(expectedShape, resultingShape, ShapeEquality::Shuffled));
	}
	{
		std::vector<SimpleBorder> resultingShape = ShapeOperations::GetUnion(preparedShape2, preparedShape1);
		ShapeOperations::OptimizeShape(resultingShape);
		EXPECT_TRUE(AreShapesEqual(expectedShape, resultingShape, ShapeEquality::Shuffled));
	}
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
	using namespace VectorUtils;
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
	using namespace VectorUtils;
	std::vector<SimpleBorder> shape1(GenerateShape({{-30.0f, 10.0f}, {-10.0f, -10.0f}, {10.0f, 10.0f}, {-10.0f, 30.0f}}));
	std::vector<SimpleBorder> shape2(GenerateShape({{10.0f, 10.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}}));
	std::vector<SimpleBorder> expectedShape = JoinVectors(shape1, shape2);

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoComplexFiguresBorderTouchingCornerOfFourBorders)
{
	using namespace VectorUtils;
	std::vector<SimpleBorder> shape1a(GenerateShape({{-30.0f, 10.0f}, {-10.0f, -10.0f}, {10.0f, 10.0f}, {-10.0f, 30.0f}}));
	std::vector<SimpleBorder> shape1b(GenerateShape({{10.0f, 10.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}}));
	std::vector<Vector2D> shape2{{10.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 30.0f}, {10.0f, 30.0f}};
	std::vector<SimpleBorder> expectedShape = JoinVectors(shape1a, GenerateShape({{10.0f, -20.0f}, {20.0f, -20.0f}, {20.0f, 0.0f}, {30.0f, -10.0f}, {50.0f, 10.0f}, {30.0f, 30.0f}, {20.0f, 20.0f}, {20.0f, 30.0f}, {10.0f, 30.0f}}));

	TestShapesUnionResultIsCorrect(JoinVectors(shape1a, shape1b), shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoComplexFiguresOneCornerAnd4OverlappingBorders)
{
	using namespace VectorUtils;
	std::vector<Vector2D> originalShape({{-20.0f, 0.0f}, {0.0f, -20.0f}, {20.0f, 0.0f}, {0.0f, 20.0f}});

	std::vector<SimpleBorder> shape1 = JoinVectors(GenerateShape(GetMovedShape(originalShape, {-20.0f, 0.0f})), GenerateShape(GetMovedShape(originalShape, {20.0f, 0.0f})));
	std::vector<SimpleBorder> shape2 = JoinVectors(GenerateShape(GetMovedShape(originalShape, {0.0f, -20.0f})), GenerateShape(GetMovedShape(originalShape, {0.0f, 20.0f})));

	std::vector<Vector2D> expectedShape({{-40.0f, 0.0f}, {0.0f, -40.0f}, {40.0f, 0.0f}, {0.0f, 40.0f}});

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedShape);
}

TEST(ShapeOperations, Union_TwoSameComplexFigures)
{
	using namespace VectorUtils;
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

TEST(ShapeOperations, Union_TwoRectsWithNotRoundCoordinates)
{
	// coordinates from an existent bug
	std::vector<Vector2D> shape1{{31.082706451416016f, -465.0944519042969f}, {31.082706451416016f, -438.5184631347656f}, {-128.373046875f, -438.5184631347656f}, {-128.373046875f, -465.0944519042969f}};
	std::vector<Vector2D> shape2{{-89.91729736328125f, -465.0944519042969f}, {-89.91729736328125f, -438.5184631347656f}, {-249.373046875f, -438.5184631347656f}, {-249.373046875f, -465.0944519042969f}};
	std::vector<Vector2D> expectedResult{{31.082706451416016f, -465.0944519042969f}, {31.082706451416016f, -438.5184631347656}, {-249.373046875f, -438.5184631347656f}, {-249.373046875f, -465.0944519042969f}};

	TestShapesUnionResultIsCorrect(shape1, shape2, expectedResult);
}

TEST(ShapeOperations, OptimizeShape_OneExtraPoint)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -40.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape, ShapeEquality::Shuffled));
}

TEST(ShapeOperations, OptimizeShape_TwoExtraPointOnABorder)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -40.0f}, {10.0f, 40.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape, ShapeEquality::Shuffled));
}

TEST(ShapeOperations, OptimizeShape_DuplicatedPoint)
{
	std::vector<SimpleBorder> shape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}});
	ShapeOperations::OptimizeShape(shape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape, ShapeEquality::Shuffled));
}

TEST(ShapeOperations, OptimizeShape_TwoFiguresTouchingWithMirroredAngle)
{
	using namespace VectorUtils;
	std::vector<SimpleBorder> tempShape1 = GenerateShape(std::vector<Vector2D>{{0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}});
	std::vector<SimpleBorder> tempShape2 = GenerateShape(std::vector<Vector2D>{{10.0f, 10.0f}, {20.0f, 10.0f}, {20.0f, 20.0f}, {10.0f, 20.0f}});
	std::vector<SimpleBorder> testShape = JoinVectors(std::move(tempShape1), std::move(tempShape2));
	// expect the shape to be unchanged, since we don't want borders with opposing directions to merge
	std::vector<SimpleBorder> expectedShape = testShape;
	ShapeOperations::OptimizeShape(testShape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, testShape, ShapeEquality::Shuffled));
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
	EXPECT_TRUE(AreShapesEqual(expectedShape, testShape, ShapeEquality::Shuffled));
}

TEST(ShapeOperations, OptimizeShape_NonConvexFigure)
{
	// borders and order taken from an existent bug
	std::vector<SimpleBorder> testShape{
		{{10.0f, 20.0f}, {-10.0f, 20.0f}},
		{{30.0f, -60.0f}, {30.0f, 10.0f}},
		{{-30.0f, 10.0f}, {-30.0f, -60.0f}},
		{{-30.0f, -60.0f}, {-10.0f, -60.0f}},
		{{-10.0f, -60.0f}, {10.0f, -60.0f}},
		{{10.0f, -60.0f}, {30.0f, -60.0f}},
		{{10.0f, 10.0f}, {10.0f, 20.0f}},
		{{-10.0f, 20.0f}, {-10.0f, 10.0f}},
		{{30.0f, 10.0f}, {10.0f, 10.0f}},
		{{-10.0f, 10.0f}, {-30.0f, 10.0f}}
	};

	std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{30.0f, -60.0f}, {30.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 20.0f}, {-10.0f, 20.0f}, {-10.0f, 10.0f}, {-30.0f, 10.0f}, {-30.0f, -60.0f}});
	ShapeOperations::OptimizeShape(testShape);
	EXPECT_TRUE(AreShapesEqual(expectedShape, testShape, ShapeEquality::Shuffled));
}

TEST(ShapeOperations, SortBorders_SortWithoutHoles)
{
	const std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{{10.0f, -60.0f}, {10.0f, 60.0f}, {-10.0f, 60.0f}, {-10.0f, -60.0f}}, ShapeOrder::Ordered);
	std::vector<SimpleBorder> shape = expectedShape;
	std::shuffle(shape.begin(), shape.end(), std::mt19937(std::random_device()()));

	std::vector<size_t> foundShapes = ShapeOperations::SortBorders(shape);

	EXPECT_EQ(1u, foundShapes.size());
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape, ShapeEquality::Ordered));
}

TEST(ShapeOperations, SortBorders_SortWithHoles)
{
	using namespace VectorUtils;
	const std::vector<SimpleBorder> outerShape = GenerateShape(std::vector<Vector2D>{{30.0f, -60.0f}, {30.0f, 60.0f}, {-30.0f, 60.0f}, {-30.0f, -60.0f}}, ShapeOrder::Ordered);
	const std::vector<SimpleBorder> holeShape = GenerateShape(std::vector<Vector2D>{{-20.0f, -40.0f}, {-20.0f, 40.0f}, {20.0f, 40.0f}}, ShapeOrder::Ordered);
	std::vector<SimpleBorder> shape = JoinVectors(outerShape, holeShape);

	std::shuffle(shape.begin(), shape.end(), std::mt19937(std::random_device()()));

	const std::vector<size_t> foundShapes = ShapeOperations::SortBorders(shape);

	EXPECT_EQ(2u, foundShapes.size());
	if (foundShapes.size() >= 2)
	{
		const size_t secondShapeStart = foundShapes[1];
		// order is not specified, so we get the order from elemets count
		bool isOuterShapeFirst = (secondShapeStart == 4);
		auto& firstExpectedShape = isOuterShapeFirst ? outerShape : holeShape;
		auto& secondExpectedShape = isOuterShapeFirst ? holeShape : outerShape;

		EXPECT_TRUE(AreShapesEqual(firstExpectedShape, {shape.begin(), secondShapeStart}, ShapeEquality::Ordered));
		EXPECT_TRUE(AreShapesEqual(secondExpectedShape, {shape.begin() + secondShapeStart, shape.end()}, ShapeEquality::Ordered));
	}
}

TEST(ShapeOperations, SortBorders_SortNonConvex)
{
	const std::vector<SimpleBorder> expectedShape = GenerateShape(std::vector<Vector2D>{
		{-30.0f, 30.0f}, {-30.0f, -10.0f}, {-20.0f, 10.0f},
		{30.0f, -10.0f}, {20.0f, -10.0f}, {10.0f, -20.0f},
		{0.0f, -10.0f}, {0.0f, -30.0f}, {20.0f, -40.0f},
		{40.0f, -10.0f}, {-10.0f, 10.0f}, {10.0f, 10.0f},
		{30.0f, 30.0f}, {-10.0f, 20.0f}
	}, ShapeOrder::Ordered);

	std::vector<SimpleBorder> shape = expectedShape;

	std::shuffle(shape.begin(), shape.end(), std::mt19937(std::random_device()()));

	std::vector<size_t> foundShapes = ShapeOperations::SortBorders(shape);

	EXPECT_EQ(1u, foundShapes.size());
	EXPECT_TRUE(AreShapesEqual(expectedShape, shape, ShapeEquality::Ordered));
}
