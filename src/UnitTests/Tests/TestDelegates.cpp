#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include <raccoon-ecs/delegates.h>


TEST(Delegates, SingleCast)
{
	RaccoonEcs::SinglecastDelegate<int> delegate;

	int testVal = 1;

	delegate.callSafe(2);
	ASSERT_THROW(delegate.callUnsafe(2), std::exception);

	delegate.assign([&testVal](const int test)
	{
		testVal = test;
	});

	ASSERT_EQ(1, testVal);

	delegate.callSafe(3);

	ASSERT_EQ(3, testVal);

	delegate.callUnsafe(4);

	ASSERT_EQ(4, testVal);

	delegate.clear();
	delegate.callSafe(5);

	ASSERT_EQ(4, testVal);

	ASSERT_THROW(delegate.callUnsafe(6), std::exception);

	ASSERT_EQ(4, testVal);
}

TEST(Delegates, MultiCast)
{
	RaccoonEcs::MulticastDelegate<int> delegate;

	int testVal1 = 1;
	int testVal2 = 1;

	delegate.broadcast(2);

	const RaccoonEcs::Delegates::Handle handle = delegate.bind([&testVal1](const int test)
	{
		testVal1 += test;
	});
	delegate.broadcast(3);

	ASSERT_EQ(4, testVal1);
	ASSERT_EQ(1, testVal2);

	delegate.bind([&testVal2](const int test)
	{
		testVal2 += test;
	});

	delegate.broadcast(4);

	ASSERT_EQ(8, testVal1);
	ASSERT_EQ(5, testVal2);

	delegate.unbind(handle);
	delegate.broadcast(5);

	ASSERT_EQ(8, testVal1);
	ASSERT_EQ(10, testVal2);

	delegate.unbind(handle);
	delegate.broadcast(5);

	ASSERT_EQ(8, testVal1);
	ASSERT_EQ(15, testVal2);

	delegate.clear();
	delegate.broadcast(4);

	ASSERT_EQ(8, testVal1);
	ASSERT_EQ(15, testVal2);
}
