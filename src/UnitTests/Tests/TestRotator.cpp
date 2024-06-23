#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/Geometry/Rotator.h"


TEST(Rotator, Comparison)
{
	const Rotator zero{0.0f};
	const Rotator pi{PI};
	const Rotator minusPi{-PI};
	const Rotator minusHalfPi{-PI/2.0f};
	const Rotator smallAngle{0.01f};

	EXPECT_TRUE((pi-pi).isNearlyEqualTo(zero));
	EXPECT_TRUE((zero-minusHalfPi-minusHalfPi).isNearlyEqualTo(minusPi));
	EXPECT_TRUE(pi.isNearlyEqualTo(minusPi));
	EXPECT_TRUE((zero-minusHalfPi-minusHalfPi).isNearlyEqualTo(pi));
	EXPECT_FALSE(zero.isNearlyEqualTo(smallAngle));
	EXPECT_FALSE(smallAngle.isNearlyEqualTo(zero));
	EXPECT_FALSE((minusPi+smallAngle).isNearlyEqualTo(pi));
	EXPECT_FALSE((minusPi-smallAngle).isNearlyEqualTo(pi));
}

TEST(Rotator, Normalization)
{
	const Rotator zero{0.0f};
	const Rotator pi{PI};
	const Rotator halfPi{PI/2.0f};
	EXPECT_TRUE(pi.isNearlyEqualTo(zero + pi + halfPi + halfPi + pi));
	EXPECT_TRUE(zero.isNearlyEqualTo(zero - pi - pi));
	EXPECT_TRUE(pi.isNearlyEqualTo(Rotator(PI*3.0f)));
	EXPECT_TRUE(halfPi.isNearlyEqualTo(Rotator(PI*6.5f)));
}
