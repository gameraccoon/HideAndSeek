#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/ComplexTypes/VectorUtils.h"

class NonCopyableClassForRemoveIndexes
{
public:
	NonCopyableClassForRemoveIndexes() = default;
	explicit NonCopyableClassForRemoveIndexes(const int value) : value(value) {}
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
		const std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
		EXPECT_EQ(expectedResult, testVector);
	}

	{
		std::vector<std::string> testVector{"0","1","2","3","4","5","6","7","8","9"};
		VectorUtils::RemoveIndexes(testVector, {3,4,7});
		const std::vector<std::string> expectedResult{"0","1","2","5","6","8","9"};
		EXPECT_EQ(expectedResult, testVector);
	}

	{
		std::vector<NonCopyableClassForRemoveIndexes> testVector;
		testVector.reserve(10);
		for (int i = 0; i < 10; ++i)
		{
			testVector.emplace_back(i);
		}
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
		const std::vector<size_t> expectedResult{0,1,2,5,6,8,9};
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

TEST(VectorUtils, JoinVectors)
{
	using namespace VectorUtils;

	EXPECT_EQ(std::vector<int>({1, 2, 3, 4}), JoinVectors(std::vector<int>({1, 2}), std::vector<int>({3, 4})));

	EXPECT_EQ(
		std::vector<int>({1, 2, 3, 4, 5}),
		JoinVectors(
			std::vector<int>({1, 2}),
			std::vector<int>({3, 4}),
			std::vector<int>({5})
		)
	);

	EXPECT_EQ(
		std::vector<int>({1, 2, 3}),
		JoinVectors(
			std::vector<int>({1, 2}),
			std::vector<int>({}),
			std::vector<int>({3})
		)
	);

	{
		std::vector<int> lvalueOriginalVector({1, 2});
		std::vector<int> movedOriginalVector({3, 4});

		EXPECT_EQ(
			std::vector<int>({1, 2, 3, 4}),
			JoinVectors(lvalueOriginalVector, std::move(movedOriginalVector))
		);

		// original lvalue vector should not change
		EXPECT_EQ(std::vector<int>({1, 2}), lvalueOriginalVector);
	}
}

TEST(VectorUtils, AppendToVector)
{
	using namespace VectorUtils;

	{
		std::vector<int> base;
		std::vector<int> appended{1, 2};
		AppendToVector(base, appended);
		EXPECT_EQ(static_cast<size_t>(2), base.size());
		EXPECT_EQ(static_cast<size_t>(2), appended.size());
		if (base.size() > 1) {
			EXPECT_EQ(1, base[0]);
			EXPECT_EQ(2, base[1]);
		}
	}

	{
		std::vector<int> base;
		std::vector<int> appended{1, 2};
		AppendToVector(base, std::move(appended));
		EXPECT_EQ(static_cast<size_t>(2), base.size());
		if (base.size() > 1) {
			EXPECT_EQ(1, base[0]);
			EXPECT_EQ(2, base[1]);
		}
	}

	{
		std::vector<int> base{1};
		std::vector<int> appended{2};
		AppendToVector(base, appended);
		EXPECT_EQ(static_cast<size_t>(2), base.size());
		EXPECT_EQ(static_cast<size_t>(1), appended.size());
		if (base.size() > 1) {
			EXPECT_EQ(1, base[0]);
			EXPECT_EQ(2, base[1]);
		}
	}

	{
		std::vector<int> base{1};
		std::vector<int> appended{2};
		AppendToVector(base, std::move(appended));
		EXPECT_EQ(static_cast<size_t>(2), base.size());
		if (base.size() > 1) {
			EXPECT_EQ(1, base[0]);
			EXPECT_EQ(2, base[1]);
		}
	}
}
