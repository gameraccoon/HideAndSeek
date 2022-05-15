#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/Debug/SubstringSearcher.h"

class TestSubstringSearchData
{
public:
	TestSubstringSearchData(int val) : value(val) {} // implicit
	TestSubstringSearchData(const TestSubstringSearchData&) = delete;
	TestSubstringSearchData& operator=(const TestSubstringSearchData&) = delete;
	TestSubstringSearchData(TestSubstringSearchData&&) = default;
	TestSubstringSearchData& operator=(TestSubstringSearchData&&) = default;

	int value = 0;
};

TEST(SubstringSearcher, BasicUsageVal)
{
	SubstringSearcher<int> searcher;

	std::vector<std::pair<std::vector<std::string>, int>> data;

	data.emplace_back(std::vector<std::string>{"testvalue"}, 10);
	data.emplace_back(std::vector<std::string>{"test.value"}, 20);
	data.emplace_back(std::vector<std::string>{"tes.tvalue"}, 30);

	searcher.construct(std::move(data));

	EXPECT_EQ(2u, searcher.findAllMatches("st").size());
	EXPECT_EQ(3u, searcher.findAllMatches("te").size());
	EXPECT_EQ(0u, searcher.findAllMatches("et").size());

	auto matches = searcher.findAllMatches("t.v");
	EXPECT_EQ(1u, matches.size());
	if (matches.size() > 0)
	{
		EXPECT_EQ(20, matches[0]);
	}
}

TEST(SubstringSearcher, BasicUsageRef)
{
	SubstringSearcher<TestSubstringSearchData> searcher;

	std::vector<std::pair<std::vector<std::string>, TestSubstringSearchData>> data;

	data.emplace_back(std::vector<std::string>{"testvalue"}, TestSubstringSearchData(10));
	data.emplace_back(std::vector<std::string>{"test.value"}, TestSubstringSearchData(20));
	data.emplace_back(std::vector<std::string>{"tes.tvalue"}, TestSubstringSearchData(30));

	searcher.construct(std::move(data));

	EXPECT_EQ(2u, searcher.findAllMatchesRef("st").size());
	EXPECT_EQ(3u, searcher.findAllMatchesRef("te").size());
	EXPECT_EQ(0u, searcher.findAllMatchesRef("et").size());

	auto matches = searcher.findAllMatchesRef("t.v");
	EXPECT_EQ(1u, matches.size());
	if (matches.size() > 0)
	{
		EXPECT_EQ(20, matches[0].get().value);
	}
}

TEST(SubstringSearcher, MultipleKeys)
{
	SubstringSearcher<int> searcher;

	std::vector<std::pair<std::vector<std::string>, int>> data;

	data.emplace_back(std::vector<std::string>{"value1", "value1alias1", "value1alias2"}, 10);
	data.emplace_back(std::vector<std::string>{"value2"}, 20);
	data.emplace_back(std::vector<std::string>{"value3", "value3alias1"}, 30);

	searcher.construct(std::move(data));

	EXPECT_EQ(1u, searcher.findAllMatches("value1").size());
	EXPECT_EQ(1u, searcher.findAllMatches("value2").size());
	EXPECT_EQ(1u, searcher.findAllMatches("value3").size());
	EXPECT_EQ(3u, searcher.findAllMatches("value").size());
	EXPECT_EQ(2u, searcher.findAllMatches("alias").size());

	auto matches = searcher.findAllMatches("alias2");
	EXPECT_EQ(1u, matches.size());
	if (matches.size() > 0)
	{
		EXPECT_EQ(10, matches[0]);
	}
}
