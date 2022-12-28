#include "Base/precomp.h"

#include <gtest/gtest.h>

#include <limits>

#include "Base/Types/String/StringNumberConversion.h"

TEST(StringNumberConversions, ParseInt_CorrectInput)
{
	EXPECT_EQ(std::optional<int>(0), String::ParseInt("0"));
	EXPECT_EQ(std::optional<int>(10), String::ParseInt("10"));
	EXPECT_EQ(std::optional<int>(7777), String::ParseInt("7777"));
	EXPECT_EQ(std::optional<int>(-666), String::ParseInt("-666"));
	// max 32bit signed int
	EXPECT_EQ(std::optional<int>(static_cast<int>(2147483647)), String::ParseInt("2147483647"));
	// min 32bit signed int
	EXPECT_EQ(std::optional<int>(static_cast<int>(-2147483648)), String::ParseInt("-2147483648"));
}

// these may work, but it's better to not rely on them
TEST(StringNumberConversions, ParseInt_SpecialInputCasesConsideredCorrect)
{
	// negative zero is still zero
	EXPECT_EQ(std::optional<int>(0), String::ParseInt("-0"));

	// for some reason this case is parsed correctly
	EXPECT_EQ(std::optional<int>(12), String::ParseInt("   12"));
}

TEST(StringNumberConversions, ParseInt_IncorrectInput)
{
	EXPECT_EQ(std::nullopt, String::ParseInt(""));
	EXPECT_EQ(std::nullopt, String::ParseInt("two"));
	EXPECT_EQ(std::nullopt, String::ParseInt("-"));
	EXPECT_EQ(std::nullopt, String::ParseInt("___"));
	EXPECT_EQ(std::nullopt, String::ParseInt("a4j6"));
	EXPECT_EQ(std::nullopt, String::ParseInt("this long string conta1ns a number"));
}

// these may fail to parse but it's better to not rely that it will always fail
TEST(StringNumberConversions, ParseInt_SpecialInputCasesConsideredIncorrect)
{
	EXPECT_EQ(std::nullopt, String::ParseInt("32   "));
	EXPECT_EQ(std::nullopt, String::ParseInt("-42   "));
	EXPECT_EQ(std::nullopt, String::ParseInt("  100  "));
	EXPECT_EQ(std::nullopt, String::ParseInt("10.0"));
	EXPECT_EQ(std::nullopt, String::ParseInt("12.5"));
	EXPECT_EQ(std::nullopt, String::ParseInt(".5"));
	EXPECT_EQ(std::nullopt, String::ParseInt("3."));
	EXPECT_EQ(std::nullopt, String::ParseInt("-1.4"));
}

TEST(StringNumberConversions, ParseIntUnsafe_CorrectInput)
{
	EXPECT_EQ(0, String::ParseIntUnsafe("0"));
	EXPECT_EQ(10, String::ParseIntUnsafe("10"));
	EXPECT_EQ(7777, String::ParseIntUnsafe("7777"));
	EXPECT_EQ(-666, String::ParseIntUnsafe("-666"));
	// max 32bit signed int
	EXPECT_EQ(2147483647, String::ParseIntUnsafe("2147483647"));
	// min 32bit signed int
	EXPECT_EQ(-2147483648, String::ParseIntUnsafe("-2147483648"));
}
