#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/String/StringHelpers.h"

TEST(FormatString, NumericalFormats)
{
	EXPECT_EQ("test10test", FormatString("test%dtest", 10));
	EXPECT_EQ("test10.00test", FormatString("test%.2ftest", 10.0f));
}

TEST(FormatString, StringFormats)
{
	EXPECT_EQ("teststrRAWtest", FormatString("test%stest", "strRAW"));
	EXPECT_EQ("teststrRWtest", FormatString("test%stest", std::string("strRW")));

	std::string testString("strLW");
	EXPECT_EQ("teststrLWtest", FormatString("test%stest", testString));

	std::string& testStringRef(testString);
	EXPECT_EQ("teststrLWtestref", FormatString("test%stestref", testStringRef));
	const std::string& testStringConstRef(testString);
	EXPECT_EQ("teststrLWtestconstref", FormatString("test%stestconstref", testStringConstRef));
}

TEST(FormatString, StringIdFormat)
{
	StringId testAStrId = STR_TO_ID("testA");
	EXPECT_EQ("testtestAtest", FormatString("test%stest", testAStrId));
}
