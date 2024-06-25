#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineCommon/Types/String/StringId.h"

TEST(StringId, TestHashingSanity)
{
	constexpr StringId valA1 = STR_TO_ID("testA");
	constexpr StringId valB1 = STR_TO_ID("testB");
	// these also should be calculated at compile time
	const StringId valA2 = STR_TO_ID("testA");
	const StringId valB2 = STR_TO_ID("testB");
	const std::string valA3 = "testA";
	const std::string valB3 = "testB";

	EXPECT_EQ(valA1, valA2);
	EXPECT_EQ(valA2, STR_TO_ID(valA3));
	EXPECT_NE(valA1, valB1);
	EXPECT_NE(valA2, valB2);
	EXPECT_NE(valA2, STR_TO_ID(valB3));
}

TEST(StringId, TestIdToString)
{
	// compile time generated StringId
	const StringId testC = STR_TO_ID("testC");
	EXPECT_EQ(std::string("testC"), ID_TO_STR(testC));

	// runtime generated StringId
	const std::string testDStr = "testD";
	const StringId testD = STR_TO_ID(testDStr);
	EXPECT_EQ(testDStr, ID_TO_STR(testD));
}
