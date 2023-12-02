#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/ComplexTypes/EnumBitset.h"

TEST(Bitset, Usage)
{
	enum class TestEnum
	{
		One, Two, Three
	};

	{
		EnumBitset<TestEnum> test{TestEnum::One};
		EXPECT_TRUE(test.has(TestEnum::One));
		EXPECT_FALSE(test.has(TestEnum::Two));
		EXPECT_FALSE(test.has(TestEnum::Three));
		EXPECT_FALSE(test.hasAnyOf(TestEnum::Two, TestEnum::Three));
		EXPECT_FALSE(test.hasAll(TestEnum::One, TestEnum::Two));

		test.set(TestEnum::Three);
		EXPECT_TRUE(test.has(TestEnum::One));
		EXPECT_FALSE(test.has(TestEnum::Two));
		EXPECT_TRUE(test.has(TestEnum::Three));
		EXPECT_TRUE(test.hasAnyOf(TestEnum::One, TestEnum::Two));
		EXPECT_FALSE(test.hasAll(TestEnum::One, TestEnum::Two));
		EXPECT_TRUE(test.hasAll(TestEnum::One, TestEnum::Three));
		EXPECT_FALSE(test.hasAll(TestEnum::One, TestEnum::Two, TestEnum::Three));
	}

	{
		EnumBitset<TestEnum> test{TestEnum::One, TestEnum::Three};
		EXPECT_TRUE(test.has(TestEnum::One));
		EXPECT_FALSE(test.has(TestEnum::Two));
		EXPECT_TRUE(test.has(TestEnum::Three));
		EXPECT_TRUE(test.hasAnyOf(TestEnum::One, TestEnum::Two));
		EXPECT_FALSE(test.hasAll(TestEnum::One, TestEnum::Two));
		EXPECT_TRUE(test.hasAll(TestEnum::One, TestEnum::Three));
		EXPECT_FALSE(test.hasAll(TestEnum::One, TestEnum::Two, TestEnum::Three));
	}
}
