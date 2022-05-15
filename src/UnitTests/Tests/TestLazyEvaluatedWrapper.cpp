#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Utils/Functional/LazyEvaluatedWrapper.h"

TEST(LazyEvaluatedWrapper, CallLambda)
{
	int runCount = 0;
	LazyEvaluated lazyEvaluatedLambda([&runCount]()
	{
		++runCount;
		return 8;
	});

	EXPECT_EQ(0, runCount);
	EXPECT_EQ(8, lazyEvaluatedLambda());
	EXPECT_EQ(1, runCount);
	EXPECT_EQ(8, lazyEvaluatedLambda());
	EXPECT_EQ(1, runCount);
}

TEST(LazyEvaluatedWrapper, CallStdFunction)
{
	int runCount = 0;
	LazyEvaluated lazyEvaluatedStdFunction(std::function([&runCount]()
	{
		++runCount;
		return 9;
	}));

	EXPECT_EQ(0, runCount);
	EXPECT_EQ(9, lazyEvaluatedStdFunction());
	EXPECT_EQ(1, runCount);
	EXPECT_EQ(9, lazyEvaluatedStdFunction());
	EXPECT_EQ(1, runCount);
}

namespace TestLazyEvaluatedWrapper
{
	static int gTestCallsCount = 0;

	static int TestFn()
	{
		++gTestCallsCount;
		return 10;
	}
}

TEST(LazyEvaluatedWrapper, CallRawFunction)
{
	using namespace TestLazyEvaluatedWrapper;

	gTestCallsCount = 0;
	LazyEvaluated lazyEvaluatedFunction(&TestFn);

	EXPECT_EQ(0, gTestCallsCount);
	EXPECT_EQ(10, lazyEvaluatedFunction());
	EXPECT_EQ(1, gTestCallsCount);
	EXPECT_EQ(10, lazyEvaluatedFunction());
	EXPECT_EQ(1, gTestCallsCount);
}

static int& LazyEvaluatedWrapperTestFnReturnRef()
{
	static int test = 0;
	return test;
}

TEST(LazyEvaluatedWrapper, ReferenceReturnType)
{
	LazyEvaluated<int&(*)(), std::reference_wrapper<int>> lazyEvaluatedFunction(&LazyEvaluatedWrapperTestFnReturnRef);

	LazyEvaluatedWrapperTestFnReturnRef() = 0;

	{
		int& test = lazyEvaluatedFunction();
		EXPECT_EQ(0, test);
		test = 1;
	}

	{
		int& test = lazyEvaluatedFunction();
		EXPECT_EQ(1, test);
	}

	static_assert(std::is_same<decltype(lazyEvaluatedFunction()), std::reference_wrapper<int>>::value, "Reference type should be returned");
}
