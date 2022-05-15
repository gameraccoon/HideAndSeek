#include "Base/precomp.h"

#include <gtest/gtest.h>

#include <vector>

#include "Base/Types/TemplateHelpers.h"

namespace TemplateHelperTestsInternal
{
	class Movable
	{
	public:
		Movable(int value) : mVal(value) {} // NOLINT(google-explicit-constructor)
		Movable(const Movable& other) = default;
		Movable& operator=(const Movable& other) = default;
		Movable(Movable&& other) noexcept : mVal(other.mVal) { other.mVal = 0; }
		Movable& operator=(Movable&& other)  noexcept { mVal = other.mVal; other.mVal = 0; return *this; }

		bool operator==(const Movable& other) const { return other.mVal == mVal; }

		friend std::ostream& operator<<(std::ostream& os, const Movable& value) {
			return os << value.mVal;
		}

	private:
		int mVal;
	};
}

TEST(TemplateHelpers, CopyOrMoveContainer)
{
	using namespace TemplateHelperTestsInternal;

	std::vector<Movable> lvalueOriginalVector({1, 2});
	std::vector<Movable> movedOriginalVector({3, 4});

	std::vector<Movable> result;
	result.reserve(4);

	TemplateHelpers::CopyOrMoveContainer<std::vector<Movable>>(lvalueOriginalVector, std::back_inserter(result));
	TemplateHelpers::CopyOrMoveContainer<std::vector<Movable>&&>(std::move(movedOriginalVector), std::back_inserter(result));

	EXPECT_EQ(std::vector<Movable>({1, 2, 3, 4}), result);
	EXPECT_EQ(std::vector<Movable>({1, 2}), lvalueOriginalVector);
	EXPECT_EQ(std::vector<Movable>({0, 0}), movedOriginalVector); // NOLINT(bugprone-use-after-move)
}
