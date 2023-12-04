#include "Base/precomp.h"

#include <gtest/gtest.h>

#include <array>
#include <algorithm>

#include "Base/Types/ComplexTypes/SimpleBitset.h"

TEST(BitsetTraits, BitsetTraits_ByteCount_ReasonableAmountOfBytes)
{
	EXPECT_EQ(static_cast<size_t>(1), BitsetTraits<1>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(1), BitsetTraits<8>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(2), BitsetTraits<9>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(2), BitsetTraits<16>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(3), BitsetTraits<17>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(3), BitsetTraits<24>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(4), BitsetTraits<25>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(4), BitsetTraits<32>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(10), BitsetTraits<80>::ByteCount);
	EXPECT_EQ(static_cast<size_t>(11), BitsetTraits<81>::ByteCount);
}

TEST(SimpleBitset, BitsetWithSize_GetBitCount_ReturnsExpectedValue)
{
	SimpleBitset<8> bitset;
	EXPECT_EQ(static_cast<size_t>(8), bitset.getBitCount());

	SimpleBitset<9> bitset2;
	EXPECT_EQ(static_cast<size_t>(9), bitset2.getBitCount());

	SimpleBitset<16> bitset3;
	EXPECT_EQ(static_cast<size_t>(16), bitset3.getBitCount());

	SimpleBitset<17> bitset4;
	EXPECT_EQ(static_cast<size_t>(17), bitset4.getBitCount());

	SimpleBitset<24> bitset5;
	EXPECT_EQ(static_cast<size_t>(24), bitset5.getBitCount());

	SimpleBitset<25> bitset6;
	EXPECT_EQ(static_cast<size_t>(25), bitset6.getBitCount());

	SimpleBitset<32> bitset7;
	EXPECT_EQ(static_cast<size_t>(32), bitset7.getBitCount());

	SimpleBitset<80> bitset8;
	EXPECT_EQ(static_cast<size_t>(80), bitset8.getBitCount());

	SimpleBitset<81> bitset9;
	EXPECT_EQ(static_cast<size_t>(81), bitset9.getBitCount());
}

TEST(SimpleBitset, BitsetWithValues_CallGet_ReturnsExpectedValues)
{
	SimpleBitset<8> bitset;
	bitset.set(0, true);
	bitset.set(1, false);
	bitset.set(2, true);
	bitset.set(3, false);
	bitset.set(4, true);
	bitset.set(5, false);
	bitset.set(6, true);
	bitset.set(7, false);

	EXPECT_TRUE(bitset.get(0));
	EXPECT_FALSE(bitset.get(1));
	EXPECT_TRUE(bitset.get(2));
	EXPECT_FALSE(bitset.get(3));
	EXPECT_TRUE(bitset.get(4));
	EXPECT_FALSE(bitset.get(5));
	EXPECT_TRUE(bitset.get(6));
	EXPECT_FALSE(bitset.get(7));
}

TEST(SimpleBitset, BitsetWithValues_Clear_ClearsAllValues)
{
	SimpleBitset<8> bitset;
	bitset.set(0, true);
	bitset.set(1, false);
	bitset.set(2, true);
	bitset.set(3, false);
	bitset.set(4, true);
	bitset.set(5, false);
	bitset.set(6, true);
	bitset.set(7, false);

	bitset.clear();

	EXPECT_FALSE(bitset.get(0));
	EXPECT_FALSE(bitset.get(1));
	EXPECT_FALSE(bitset.get(2));
	EXPECT_FALSE(bitset.get(3));
	EXPECT_FALSE(bitset.get(4));
	EXPECT_FALSE(bitset.get(5));
	EXPECT_FALSE(bitset.get(6));
	EXPECT_FALSE(bitset.get(7));
}

TEST(SimpleBitset, TwoBitsetsWithValues_Intersect_OnlyCommonValuesAreSet)
{
	SimpleBitset<4> bitset1;
	bitset1.set(0, false);
	bitset1.set(1, true);
	bitset1.set(2, false);
	bitset1.set(3, true);

	SimpleBitset<4> bitset2;
	bitset2.set(0, false);
	bitset2.set(1, false);
	bitset2.set(2, true);
	bitset2.set(3, true);

	bitset1.intersect(bitset2);

	EXPECT_FALSE(bitset1.get(0));
	EXPECT_FALSE(bitset1.get(1));
	EXPECT_FALSE(bitset1.get(2));
	EXPECT_TRUE(bitset1.get(3));
}

TEST(SimpleBitset, TwoBitsetsWithValues_Unite_AllValuesAreSet)
{
	SimpleBitset<4> bitset1;
	bitset1.set(0, false);
	bitset1.set(1, true);
	bitset1.set(2, false);
	bitset1.set(3, true);

	SimpleBitset<4> bitset2;
	bitset2.set(0, false);
	bitset2.set(1, false);
	bitset2.set(2, true);
	bitset2.set(3, true);

	bitset1.unite(bitset2);

	EXPECT_FALSE(bitset1.get(0));
	EXPECT_TRUE(bitset1.get(1));
	EXPECT_TRUE(bitset1.get(2));
	EXPECT_TRUE(bitset1.get(3));
}

TEST(SimpleBitset, BitsetWithValues_Invert_InvertsAllValues)
{
	SimpleBitset<8> bitset;
	bitset.set(0, true);
	bitset.set(1, false);
	bitset.set(2, true);
	bitset.set(3, false);
	bitset.set(4, true);
	bitset.set(5, false);
	bitset.set(6, true);
	bitset.set(7, false);

	bitset.invert();

	EXPECT_FALSE(bitset.get(0));
	EXPECT_TRUE(bitset.get(1));
	EXPECT_FALSE(bitset.get(2));
	EXPECT_TRUE(bitset.get(3));
	EXPECT_FALSE(bitset.get(4));
	EXPECT_TRUE(bitset.get(5));
	EXPECT_FALSE(bitset.get(6));
	EXPECT_TRUE(bitset.get(7));
}

TEST(SimpleBitset, BitsetWithValues_GetRawDataAndThenSetRawData_ProducesSameValues)
{
	std::array<std::byte, BitsetTraits<8>::ByteCount> rawData;
	{
		SimpleBitset<8> bitset;
		bitset.set(0, true);
		bitset.set(1, false);
		bitset.set(2, true);
		bitset.set(3, false);
		bitset.set(4, true);
		bitset.set(5, false);
		bitset.set(6, true);
		bitset.set(7, false);

		std::copy(bitset.getRawData(), bitset.getRawData() + BitsetTraits<8>::ByteCount, rawData.data());
	}
	SimpleBitset<8> bitset2;
	bitset2.setRawData(rawData.data());

	EXPECT_TRUE(bitset2.get(0));
	EXPECT_FALSE(bitset2.get(1));
	EXPECT_TRUE(bitset2.get(2));
	EXPECT_FALSE(bitset2.get(3));
	EXPECT_TRUE(bitset2.get(4));
	EXPECT_FALSE(bitset2.get(5));
	EXPECT_TRUE(bitset2.get(6));
	EXPECT_FALSE(bitset2.get(7));
}

// make sure we don't depend on endianness, and this test doesn't fail on some platforms with the same data
TEST(SimpleBitset, RawValues_CreateBitsetWithRawData_ProducesExpectedValues)
{
	std::array<std::byte, BitsetTraits<8>::ByteCount> rawData;
	rawData[0] = std::byte(0xF7);

	SimpleBitset<8> bitset;
	bitset.setRawData(rawData.data());

	EXPECT_TRUE(bitset.get(0));
	EXPECT_TRUE(bitset.get(1));
	EXPECT_TRUE(bitset.get(2));
	EXPECT_FALSE(bitset.get(3));
	EXPECT_TRUE(bitset.get(4));
	EXPECT_TRUE(bitset.get(5));
	EXPECT_TRUE(bitset.get(6));
	EXPECT_TRUE(bitset.get(7));
}

TEST(SimpleBitset, BitsetWithSize_GetByteCount_ReturnsExpectedValue)
{
	SimpleBitset<8> bitset;
	EXPECT_EQ(static_cast<size_t>(1), bitset.getByteCount());

	SimpleBitset<9> bitset2;
	EXPECT_EQ(static_cast<size_t>(2), bitset2.getByteCount());

	SimpleBitset<16> bitset3;
	EXPECT_EQ(static_cast<size_t>(2), bitset3.getByteCount());

	SimpleBitset<17> bitset4;
	EXPECT_EQ(static_cast<size_t>(3), bitset4.getByteCount());

	SimpleBitset<24> bitset5;
	EXPECT_EQ(static_cast<size_t>(3), bitset5.getByteCount());

	SimpleBitset<25> bitset6;
	EXPECT_EQ(static_cast<size_t>(4), bitset6.getByteCount());

	SimpleBitset<32> bitset7;
	EXPECT_EQ(static_cast<size_t>(4), bitset7.getByteCount());

	SimpleBitset<80> bitset8;
	EXPECT_EQ(static_cast<size_t>(10), bitset8.getByteCount());

	SimpleBitset<81> bitset9;
	EXPECT_EQ(static_cast<size_t>(11), bitset9.getByteCount());
}
