#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/String/StringId.h"

TEST(StringId, TestHashingSanity)
{
	constexpr StringId valA1 = STR_TO_ID("testA");
	constexpr StringId valB1 = STR_TO_ID("testB");
	// these also should be calculated at compile time
	StringId valA2 = STR_TO_ID("testA");
	StringId valB2 = STR_TO_ID("testB");
	std::string valA3 = "testA";
	std::string valB3 = "testB";

	EXPECT_EQ(valA1, valA2);
	EXPECT_EQ(valA2, STR_TO_ID(valA3));
	EXPECT_NE(valA1, valB1);
	EXPECT_NE(valA2, valB2);
	EXPECT_NE(valA2, STR_TO_ID(valB3));
}

TEST(StringId, TestIdToString)
{
	// compile time generated StringId
	StringId testC = STR_TO_ID("testC");
	EXPECT_EQ(std::string("testC"), ID_TO_STR(testC));

	// runtime generated StringId
	std::string testDStr = "testD";
	StringId testD = STR_TO_ID(testDStr);
	EXPECT_EQ(testDStr, ID_TO_STR(testD));
}
