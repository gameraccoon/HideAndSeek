#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "ECS/Delegates.h"


TEST(Delegates, SingleCast)
{
	SinglecastDelegate<int> delegate;

	int testVal = 1;

	delegate.callSafe(2);
	ASSERT_THROW(delegate.callUnsafe(2), std::exception);

	delegate.assign([&testVal](int test)
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
	MulticastDelegate<int> delegate;

	int testVal1 = 1;
	int testVal2 = 1;

	delegate.broadcast(2);

	Delegates::Handle handle = delegate.bind([&testVal1](int test)
	{
		testVal1 += test;
	});
	delegate.broadcast(3);

	ASSERT_EQ(4, testVal1);
	ASSERT_EQ(1, testVal2);

	delegate.bind([&testVal2](int test)
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
