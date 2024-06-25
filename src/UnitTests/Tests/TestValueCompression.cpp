#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineCommon/Math/ValueCompression.h"


TEST(ValueCompression, FloatToIntRL)
{
	constexpr float zero = 0;
	constexpr float half = 0.5;
	constexpr float one = 1;

	constexpr unsigned int bitsCount = 16;

	const unsigned int zeroInt = Utils::CompressNormalizedFloatToIntCL<unsigned int>(zero, bitsCount);
	const unsigned int halfInt = Utils::CompressNormalizedFloatToIntCL<unsigned int>(half, bitsCount);
	const unsigned int oneInt = Utils::CompressNormalizedFloatToIntCL<unsigned int>(one, bitsCount);

	EXPECT_EQ(0u, zeroInt);
	EXPECT_EQ((1ul << bitsCount) - 1ul, oneInt);

	EXPECT_GT(halfInt, zeroInt);
	EXPECT_GT(oneInt, halfInt);

	EXPECT_FLOAT_EQ(zero, Utils::DecompressNormalizedFloatFromIntCL<float>(zeroInt, bitsCount));
	EXPECT_NEAR(half, Utils::DecompressNormalizedFloatFromIntCL<float>(halfInt, bitsCount), 0.001);
	EXPECT_FLOAT_EQ(one, Utils::DecompressNormalizedFloatFromIntCL<float>(oneInt, bitsCount));
}
