#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "GameUtils/Functional/ScopeFinalizer.h"

TEST(ScopeFinalizer, CallLambda)
{
	int runCount = 0;
	{
		auto _ = ScopeFinalizer([&runCount]()
		{
			++runCount;
		});
		EXPECT_EQ(0, runCount);
	}
	EXPECT_EQ(1, runCount);
}

TEST(ScopeFinalizer, CallStdFunction)
{
	int runCount = 0;
	{
		auto _ = ScopeFinalizer(std::function([&runCount]()
		{
			++runCount;
		}));
		EXPECT_EQ(0, runCount);
	}
	EXPECT_EQ(1, runCount);
}

namespace TestScopeFinalizerInternals
{
	static int gTestCallsCount = 0;

	static void TestFn()
	{
		++gTestCallsCount;
	}
}

TEST(ScopeFinalizer, CallRawFunction)
{
	using namespace TestScopeFinalizerInternals;

	gTestCallsCount = 0;
	{
		auto _ = ScopeFinalizer(&TestFn);
		EXPECT_EQ(0, gTestCallsCount);
	}
	EXPECT_EQ(1, gTestCallsCount);
}

