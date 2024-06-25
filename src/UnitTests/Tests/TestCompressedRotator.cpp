#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineCommon/Types/BasicTypes.h"

#include "EngineData/Geometry/CompressedRotator.h"

TEST(CompressedRotator, Value)
{
	const Rotator zeroRotator{0.0f};
	const Rotator minusPiRotator{-PI};
	const Rotator minusHalfPiRotator{-PI/2.0f};
	const Rotator piRotator{PI};

	EXPECT_TRUE(zeroRotator.isNearlyEqualTo(CompressedRotator<u16>{zeroRotator}.decompress()));
	EXPECT_TRUE(minusPiRotator.isNearlyEqualTo(CompressedRotator<u16>{minusPiRotator}.decompress()));
	EXPECT_TRUE(minusHalfPiRotator.isNearlyEqualTo(CompressedRotator<u16>{minusHalfPiRotator}.decompress()));
	EXPECT_TRUE(piRotator.isNearlyEqualTo(CompressedRotator<u16>{piRotator}.decompress()));

	static_assert(std::is_trivial<CompressedRotator<u16>>(), "CompressedRotator should be trivial type");
	static_assert(std::is_standard_layout<CompressedRotator<u16>>(), "CompressedRotator should have standard layout");
}

TEST(CompressedRotator, Static)
{
	const Rotator zeroRotator{0.0f};
	const Rotator minusPiRotator{-PI};
	const Rotator minusHalfPiRotator{-PI/2.0f};
	const Rotator piRotator{PI};

	const u16 zeroInt = CompressedRotator<u16>::Compress(zeroRotator, 16);
	const u16 minusPiInt = CompressedRotator<u16>::Compress(minusPiRotator, 16);
	const u16 minusHalfPiInt = CompressedRotator<u16>::Compress(minusHalfPiRotator, 16);
	const u16 piInt = CompressedRotator<u16>::Compress(piRotator, 16);

	EXPECT_EQ(0u, zeroInt);

	EXPECT_TRUE(zeroRotator.isNearlyEqualTo(CompressedRotator<u16>::Decompress(zeroInt)));
	EXPECT_TRUE(minusPiRotator.isNearlyEqualTo(CompressedRotator<u16>::Decompress(minusPiInt)));
	EXPECT_TRUE(minusHalfPiRotator.isNearlyEqualTo(CompressedRotator<u16>::Decompress(minusHalfPiInt)));
	EXPECT_TRUE(piRotator.isNearlyEqualTo(CompressedRotator<u16>::Decompress(piInt)));
}
