#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineCommon/Types/ComplexTypes/Result.h"

TEST(Result, ResultWithValue_CheckForValue_ValueExists)
{
	const Result<int, std::string> result = Result<int, std::string>::Ok(42);

	EXPECT_TRUE(result.hasValue());
}

TEST(Result, ResultWithValue_GetValue_ValueReturned)
{
	const Result<int, std::string> result = Result<int, std::string>::Ok(42);

	EXPECT_EQ(42, result.getValue());
}

TEST(Result, ResultWithValue_GetValueOr_ValueReturned)
{
	const Result<int, std::string> result = Result<int, std::string>::Ok(42);

	EXPECT_EQ(42, result.getValueOr(12));
}


TEST(Result, ResultWithValue_CompareWithOther_OnlyTheSameValueEquals)
{
	const Result<int, std::string> result = Result<int, std::string>::Ok(42);
	const Result<int, std::string> equalResult = Result<int, std::string>::Ok(42);
	const Result<int, std::string> nonEqual1 = Result<int, std::string>::Ok(12);
	const Result<int, std::string> nonEqual2 = Result<int, std::string>::Err("Error");

	EXPECT_TRUE(result == equalResult);
	EXPECT_FALSE(result != equalResult);
	EXPECT_FALSE(result == nonEqual1);
	EXPECT_TRUE(result != nonEqual1);
	EXPECT_FALSE(result == nonEqual2);
	EXPECT_TRUE(result != nonEqual2);
}

TEST(Result, ResultWithError_CompareWithOther_OnlyTheSameValueEquals)
{
	const Result<int, std::string> result = Result<int, std::string>::Err("Error");
	const Result<int, std::string> equalResult = Result<int, std::string>::Err("Error");
	const Result<int, std::string> nonEqual1 = Result<int, std::string>::Err("Another error");
	const Result<int, std::string> nonEqual2 = Result<int, std::string>::Ok(42);

	EXPECT_TRUE(result == equalResult);
	EXPECT_FALSE(result != equalResult);
	EXPECT_FALSE(result == nonEqual1);
	EXPECT_TRUE(result != nonEqual1);
	EXPECT_FALSE(result == nonEqual2);
	EXPECT_TRUE(result != nonEqual2);
}

TEST(Result, ResultWithValue_CheckForError_NoError)
{
	const Result<int, std::string> result = Result<int, std::string>::Ok(42);

	EXPECT_FALSE(result.hasError());
}

TEST(Result, ResultWithError_CheckForError_ErrorExists)
{
	const Result<int, std::string> result = Result<int, std::string>::Err("Error");

	EXPECT_TRUE(result.hasError());
}

TEST(Result, ResultWithError_GetError_ErrorReturned)
{
	const Result<int, std::string> result = Result<int, std::string>::Err("Error");

	EXPECT_EQ("Error", result.getError());
}

TEST(Result, ResultWithError_CheckForValue_NoValue)
{
	const Result<int, std::string> result = Result<int, std::string>::Err("Error");

	EXPECT_FALSE(result.hasValue());
}

TEST(Result, ResultWithError_GetValueOr_DefaultReturned)
{
	const Result<int, std::string> result = Result<int, std::string>::Err("Error");

	EXPECT_EQ(42, result.getValueOr(42));
}


TEST(Result, ResultWithNonCopyableValue_ConsumeValue_ValueIsExact)
{
	std::unique_ptr<int> value = std::make_unique<int>(42);
	Result<std::unique_ptr<int>, std::string> result = Result<std::unique_ptr<int>, std::string>::Ok(std::move(value));
	ASSERT_EQ(nullptr, value);

	const std::unique_ptr<int> newValue = result.consumeValue();
	ASSERT_EQ(nullptr, result.getValue());
	EXPECT_EQ(42, *newValue);
}
