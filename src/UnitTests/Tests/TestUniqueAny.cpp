#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <vector>

#include "EngineCommon/Types/ComplexTypes/UniqueAny.h"

TEST(UniqueAny, DefaultConstructed)
{
	UniqueAny any;
	EXPECT_EQ(nullptr, any.cast<int>());
	EXPECT_EQ(nullptr, any.cast<std::vector<int>>());
}

TEST(UniqueAny, MoveCopyConstructed)
{
	{
		UniqueAny any{UniqueAny::Create<int>(10)};
		ASSERT_NE(nullptr, any.cast<int>());
		EXPECT_EQ(nullptr, any.cast<float>());
		EXPECT_EQ(10, *any.cast<int>());
	}

	{
		// use vector constuctor that takes size and default value
		UniqueAny any{UniqueAny::Create<std::vector<int>>(3, 2)};
		ASSERT_NE(nullptr, any.cast<std::vector<int>>());
		EXPECT_EQ(nullptr, any.cast<int>());
		EXPECT_EQ(std::vector<int>({2, 2, 2}), *any.cast<std::vector<int>>());
	}
}

TEST(UniqueAny, MoveAssignConstructed)
{
	UniqueAny any;

	any = UniqueAny::Create<int>(20);
	ASSERT_NE(nullptr, any.cast<int>());
	EXPECT_EQ(nullptr, any.cast<std::vector<int>>());
	EXPECT_EQ(20, *any.cast<int>());

	// use vector constuctor that takes size and default value
	any = UniqueAny::Create<std::vector<int>>(3, 2);
	ASSERT_NE(nullptr, any.cast<std::vector<int>>());
	EXPECT_EQ(nullptr, any.cast<int>());
	EXPECT_EQ(std::vector<int>({2, 2, 2}), *any.cast<std::vector<int>>());
}


TEST(UniqueAny, CastFromConst)
{
	const UniqueAny any = UniqueAny::Create<int>(2);

	auto value = any.cast<int>();

	static_assert(!std::is_const_v<decltype(*value)>, "UniqueAny::cast called on const object should return pointer to const");
	ASSERT_NE(nullptr, value);
	EXPECT_EQ(2, *value);
}

static_assert(!std::is_copy_constructible_v<UniqueAny>, "UniqueAny should not be copy constructible");
static_assert(!std::is_copy_assignable_v<UniqueAny>, "UniqueAny should not be copy assignable");
