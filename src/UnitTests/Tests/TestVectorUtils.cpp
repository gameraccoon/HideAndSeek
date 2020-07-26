#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/ComplexTypes/VectorUtils.h"

class NonCopyableClassForRemoveIndexes
{
public:
	NonCopyableClassForRemoveIndexes() = default;
	NonCopyableClassForRemoveIndexes(int value) : value(value) {}
	NonCopyableClassForRemoveIndexes(const NonCopyableClassForRemoveIndexes&) = delete;
	NonCopyableClassForRemoveIndexes& operator=(const NonCopyableClassForRemoveIndexes&) = delete;
	NonCopyableClassForRemoveIndexes(NonCopyableClassForRemoveIndexes&&) = default;
	NonCopyableClassForRemoveIndexes& operator=(NonCopyableClassForRemoveIndexes&&) = default;

	bool operator==(const NonCopyableClassForRemoveIndexes& other) const { return value == other.value; }
	bool operator<(const NonCopyableClassForRemoveIndexes& other) const { return  value < other.value; }

	int value = 0;
};

template <typename Vec1, typename Vec2>
bool ContainSameElements(Vec1&& first, Vec2&& second)
{
	Vec1 firstSortedCopy = std::forward<Vec1>(first);
	Vec1 secondSortedCopy = std::forward<Vec2>(second);

	std::sort(firstSortedCopy.begin(), firstSortedCopy.end());
	std::sort(secondSortedCopy.begin(), secondSortedCopy.end());

	return firstSortedCopy == secondSortedCopy;
}

TEST(VectorUtils, RemoveIndexes)
{
	{
		std::vector<size_t> testVector{0,1,2,3,4,5,6,7,8,9};
		VectorUtils::RemoveIndexes(testVector, {3,4,7});
		std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
		EXPECT_EQ(expectedResult, testVector);
	}

	{
		std::vector<std::string> testVector{"0","1","2","3","4","5","6","7","8","9"};
		VectorUtils::RemoveIndexes(testVector, {3,4,7});
		std::vector<std::string> expectedResult{"0","1","2","5","6","8","9"};
		EXPECT_EQ(expectedResult, testVector);
	}

	{
		std::vector<NonCopyableClassForRemoveIndexes> testVector;
		for (int i = 0; i < 10; ++i) { testVector.emplace_back(i); }
		VectorUtils::RemoveIndexes(testVector, {3,4,7});

		std::vector<NonCopyableClassForRemoveIndexes> expectedResult;
		expectedResult.emplace_back(0);
		expectedResult.emplace_back(1);
		expectedResult.emplace_back(2);
		expectedResult.emplace_back(5);
		expectedResult.emplace_back(6);
		expectedResult.emplace_back(8);
		expectedResult.emplace_back(9);

		EXPECT_EQ(expectedResult, testVector);
	}

	{
		std::vector<size_t> testVector{0,1,2,3,4,5,6,7,8,9};
		VectorUtils::RemoveIndexes(testVector, {3,4,4,7});
		std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
		EXPECT_EQ(expectedResult, testVector);
	}
}

TEST(VectorUtils, RemoveIndexesUnordered)
{
	{
		std::vector<size_t> testVector{0,1,2,3,4,5,6,7,8,9};
		VectorUtils::RemoveIndexesFast(testVector, {3,4,7});
		std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
		EXPECT_TRUE(ContainSameElements(std::move(expectedResult), std::move(testVector)));
	}

	{
		std::vector<std::string> testVector{"0","1","2","3","4","5","6","7","8","9"};
		VectorUtils::RemoveIndexesFast(testVector, {3,4,7});
		std::vector<std::string> expectedResult{"0","1","2","5","6","8","9"};
		EXPECT_TRUE(ContainSameElements(std::move(expectedResult), std::move(testVector)));
	}

	{
		std::vector<NonCopyableClassForRemoveIndexes> testVector;
		for (int i = 0; i < 10; ++i) { testVector.emplace_back(i); }
		VectorUtils::RemoveIndexesFast(testVector, {3,4,7});

		std::vector<NonCopyableClassForRemoveIndexes> expectedResult;
		expectedResult.emplace_back(0);
		expectedResult.emplace_back(1);
		expectedResult.emplace_back(2);
		expectedResult.emplace_back(5);
		expectedResult.emplace_back(6);
		expectedResult.emplace_back(8);
		expectedResult.emplace_back(9);

		EXPECT_TRUE(ContainSameElements(std::move(expectedResult), std::move(testVector)));
	}

	{
		std::vector<size_t> testVector{0,1,2,3,4,5,6,7,8,9};
		VectorUtils::RemoveIndexesFast(testVector, {3,4,4,7});
		std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
		EXPECT_TRUE(ContainSameElements(std::move(expectedResult), std::move(testVector)));
	}
}
