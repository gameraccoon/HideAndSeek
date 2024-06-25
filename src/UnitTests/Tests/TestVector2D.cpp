#include "EngineCommon/precomp.h"

#include <map>

#include <gtest/gtest.h>

#include "EngineData/Geometry/Vector2D.h"


TEST(Vector2D, Creation)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);
	constexpr Vector2D testVectorB(testVectorA);

	EXPECT_FLOAT_EQ(testVectorA.x, 5.2f);
	EXPECT_FLOAT_EQ(testVectorA.y, -0.39f);
	EXPECT_FLOAT_EQ(testVectorB.x, 5.2f);
	EXPECT_FLOAT_EQ(testVectorB.y, -0.39f);
}

TEST(Vector2D, InitializationFromAngle)
{
	Rotator angle(0.0f);
	Vector2D testVectorA(angle);

	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(1.0, 0.0)));

	angle = Rotator(PI * 0.5f);
	testVectorA = Vector2D(angle);

	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(0.0, 1.0)));
}

TEST(Vector2D, Comparison)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);
	constexpr Vector2D testVectorB(testVectorA);
	constexpr Vector2D testVectorC(5.199998f, -0.390001f);
	constexpr Vector2D testVectorD(5.3f, -0.39f);

	EXPECT_TRUE(testVectorA == testVectorB); // A == B
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(testVectorB)); // A == B
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(testVectorC)); // A == C
	EXPECT_FALSE(testVectorA.isNearlyEqualTo(testVectorD)); // A != D
}

TEST(Vector2D, Protection)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);

	Vector2D testVectorB = testVectorA;
	testVectorB.x = 30.0f;

	EXPECT_FLOAT_EQ(30.0f, testVectorB.x);
	EXPECT_FLOAT_EQ(5.2f, testVectorA.x);
}

TEST(Vector2D, Size)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);

	const float testVectorSize = testVectorA.size();

	EXPECT_NEAR(5.2146f, testVectorSize, 0.00001);
}

TEST(Vector2D, QSize)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);

	const float testVectorSize = testVectorA.qSize();

	EXPECT_FLOAT_EQ(27.1921f, testVectorSize);
}

TEST(Vector2D, Negation)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);

	Vector2D testVectorB = -testVectorA;

	EXPECT_TRUE(testVectorB.isNearlyEqualTo(Vector2D(-5.2f, 0.39f)));

	testVectorB.y = 70.0f;

	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(5.2f, -0.39f)));
	EXPECT_FLOAT_EQ(5.2f, testVectorA.x);
}

TEST(Vector2D, Addition)
{
	Vector2D testVectorA(5.2f, -0.39f);
	Vector2D testVectorB(16.9f, 803.27f);

	const Vector2D testVectorC = testVectorA + testVectorB;
	testVectorA = testVectorB + testVectorC;
	testVectorB += testVectorB;

	EXPECT_TRUE(testVectorC.isNearlyEqualTo(Vector2D(22.1f, 802.88f)));
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(39, 1606.15f)));
	EXPECT_TRUE(testVectorB.isNearlyEqualTo(Vector2D(33.8f, 1606.54f)));
}

TEST(Vector2D, Substraction)
{
	Vector2D testVectorA(5.2f, -0.39f);
	constexpr Vector2D testVectorB(16.9f, 803.27f);

	const Vector2D testVectorC = testVectorA - testVectorB;
	testVectorA = testVectorB - testVectorC;

	EXPECT_TRUE(testVectorC.isNearlyEqualTo(Vector2D(-11.7f, -803.66f)));
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(28.6f, 1606.9301f)));
}

TEST(Vector2D, Multiplication)
{
	Vector2D testVectorA(5.2f, -0.39f);
	constexpr float testScalar = 18.3f;

	const Vector2D testVectorB = testVectorA * testScalar;
	testVectorA *= -testScalar;

	EXPECT_TRUE(testVectorB.isNearlyEqualTo(Vector2D(95.16f, -7.137f)));
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(-95.16f, 7.137f)));
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(-testVectorB));
}

TEST(Vector2D, Division)
{
	Vector2D testVectorA(5.2f, -0.39f);
	constexpr float testScalar = 18.3f;

	testVectorA /= testScalar;
	const Vector2D testVectorB = testVectorA / testVectorA.size(); // B = ort(A);

	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(0.2841f, -0.0213f)));
	EXPECT_TRUE(testVectorB.isNearlyEqualTo(Vector2D(0.9972f, -0.07476377f)));
	EXPECT_FLOAT_EQ(1.0f, testVectorB.size()); // ort size is 1.0
}

TEST(Vector2D, Unit)
{
	Vector2D testVectorA(5.2f, -0.39f);

	testVectorA = testVectorA.unit();

	EXPECT_TRUE(testVectorA.isNearlyEqualTo(Vector2D(0.9972f, -0.07476377f)));
	EXPECT_FLOAT_EQ(1.0f, testVectorA.size()); // ort size is 1.0

	testVectorA = Vector2D(0.0f, 0.0f);
	EXPECT_TRUE(testVectorA.unit().isNearlyEqualTo(Vector2D(0.0f, 0.0f)));
}

TEST(Vector2D, GetRotation)
{
	constexpr Vector2D testVectorA(1.0f, 0.0f);
	constexpr Vector2D testVectorB(-1.0f, 0.0f);
	constexpr Vector2D testVectorC(0.0f, 1.0f);
	constexpr Vector2D testVectorD(0.0f, -1.0f);
	constexpr Vector2D testVectorE(0.0f, 0.0f);

	EXPECT_FLOAT_EQ(0.0f, testVectorA.rotation().getValue());
	EXPECT_FLOAT_EQ(PI, testVectorB.rotation().getValue());
	EXPECT_FLOAT_EQ(PI/2, testVectorC.rotation().getValue());
	EXPECT_FLOAT_EQ(-PI/2, testVectorD.rotation().getValue());
	EXPECT_FLOAT_EQ(0.0f, testVectorE.rotation().getValue());		//Invalid parameter should not cause errors
}

TEST(Vector2D, DotProduct)
{
	constexpr Vector2D testVectorA(5.2f, -0.39f);
	constexpr Vector2D testVectorB(16.9f, 803.27f);

	EXPECT_FLOAT_EQ(-225.3953f, Vector2D::DotProduct(testVectorA, testVectorB));
}

TEST(Vector2D, Project)
{
	constexpr Vector2D oX(1.0f, 0.0f);
	constexpr Vector2D oYxFive(0.0f, 5.0f);

	constexpr Vector2D testVectorA(5.2f, -0.39f);
	constexpr Vector2D testVectorB(16.9f, 803.27f);

	// Another (slower) way to calc projection vector
	const Vector2D projectAB = testVectorA.unit() * testVectorB.size() * std::cos((testVectorA.rotation() - testVectorB.rotation()).getValue());

	EXPECT_TRUE(testVectorA.project(oX).isNearlyEqualTo(Vector2D(5.2f, 0.0f)));
	EXPECT_TRUE(testVectorB.project(oYxFive).isNearlyEqualTo(Vector2D(0.0f, 803.27f)));
	EXPECT_TRUE(oYxFive.project(oX).isNearlyEqualTo(Vector2D(0.0f, 0.0f)));
	EXPECT_TRUE(testVectorA.isNearlyEqualTo(testVectorA.project(oX) + testVectorA.project(oYxFive)));

	EXPECT_TRUE(projectAB.isNearlyEqualTo(testVectorB.project(testVectorA)));
}

TEST(Vector2D, Normal)
{
	EXPECT_TRUE(Vector2D(1.0f, 0.0f).isNearlyEqualTo(Vector2D(0.0f, 500.0f).normal()));
	EXPECT_TRUE(Vector2D(-0.0748f, -0.99719f).isNearlyEqualTo(Vector2D(5.2f, -0.39f).normal()));
	EXPECT_TRUE(Vector2D(-0.9998f, -0.021f).isNearlyEqualTo(Vector2D(16.9f, -803.27f).normal()));

	constexpr Vector2D testVector(-1234.321f, 184.99f);
	EXPECT_TRUE(Vector2D(testVector.rotation() - Rotator(HALF_PI)).isNearlyEqualTo(testVector.normal()));
}

TEST(Vector2D, Rotate)
{
	constexpr Vector2D testVectorA(30, 40);

	EXPECT_TRUE(Vector2D(30, 40).isNearlyEqualTo(testVectorA.getRotated(Rotator(0.0f))));
	EXPECT_TRUE(Vector2D(-40, 30).isNearlyEqualTo(testVectorA.getRotated(Rotator(PI * 0.5f))));
	EXPECT_TRUE(Vector2D(-30, -40).isNearlyEqualTo(testVectorA.getRotated(Rotator(PI))));
	EXPECT_TRUE(Vector2D(40, -30).isNearlyEqualTo(testVectorA.getRotated(Rotator(PI * 1.5f))));
	EXPECT_TRUE(Vector2D(30, 40).isNearlyEqualTo(testVectorA.getRotated(Rotator(PI * 2.0f))));
}

TEST(Vector2D, HadamardProduct)
{
	constexpr Vector2D testVectorA(10.0f, 20.0f);
	constexpr Vector2D testVectorB(-3.0f, 2.0f);

	EXPECT_TRUE(Vector2D(-30, 40).isNearlyEqualTo(Vector2D::HadamardProduct(testVectorA, testVectorB)));
}

TEST(Vector2D, Lerp)
{
	{
		constexpr Vector2D testVectorA(10.0f, 20.0f);
		constexpr Vector2D testVectorB(-3.0f, 2.0f);

		EXPECT_TRUE(Vector2D(10.0f, 20.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 0.0f)));
		EXPECT_TRUE(Vector2D(-3.0f, 2.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 1.0f)));
		EXPECT_TRUE(Vector2D(3.5f, 11.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 0.5f)));
	}

	{
		constexpr Vector2D testVectorA(10.0f, 10.0f);
		constexpr Vector2D testVectorB(10.0f, 10.0f);

		EXPECT_TRUE(Vector2D(10.0f, 10.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 0.0f)));
		EXPECT_TRUE(Vector2D(10.0f, 10.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 1.0f)));
		EXPECT_TRUE(Vector2D(10.0f, 10.0f).isNearlyEqualTo(Vector2D::Lerp(testVectorA, testVectorB, 0.5f)));
	}
}

TEST(Vector2D, InvLerp)
{
	{
		constexpr Vector2D testVectorA(10.0f, 20.0f);
		constexpr Vector2D testVectorB(-3.0f, 2.0f);

		EXPECT_FLOAT_EQ(0.0f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.0f)));
		EXPECT_FLOAT_EQ(1.0f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 1.0f)));
		EXPECT_FLOAT_EQ(0.3f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.3f)));
		EXPECT_FLOAT_EQ(0.9f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.9f)));
	}

	{
		constexpr Vector2D testVectorA(10.000001f, 10.0f);
		constexpr Vector2D testVectorB(10.0f, -20.0f);

		EXPECT_FLOAT_EQ(0.0f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.0f)));
		EXPECT_FLOAT_EQ(1.0f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 1.0f)));
		EXPECT_FLOAT_EQ(0.3f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.3f)));
		EXPECT_FLOAT_EQ(0.9f, Vector2D::InvLerp(testVectorA, testVectorB, Vector2D::Lerp(testVectorA, testVectorB, 0.9f)));
	}
}

TEST(Vector2DKey, UseInMap)
{
	std::unordered_map<Vector2DKey<>, int> vectorToInt;
	vectorToInt[Vector2DKey(Vector2D(10.0f, 10.0f))] = 1;
	vectorToInt[Vector2DKey(Vector2D(10.00001f, 10.0f))] = 2; // overwrites the first value
	vectorToInt[Vector2DKey(Vector2D(10.1f, 10.0f))] = 3;

	EXPECT_EQ(2u, vectorToInt.size());
	EXPECT_EQ(2, vectorToInt[Vector2DKey(Vector2D(10.0f, 10.0f))]);
	EXPECT_EQ(2, vectorToInt[Vector2DKey(Vector2D(10.00001f, 10.0f))]);
	EXPECT_EQ(2, vectorToInt[Vector2DKey(Vector2D(10.0f, 10.00001f))]);
	EXPECT_EQ(3, vectorToInt[Vector2DKey(Vector2D(10.1f, 10.0f))]);
}
