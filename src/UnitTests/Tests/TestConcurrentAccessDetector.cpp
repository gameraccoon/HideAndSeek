#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "UnitTests/TestAssertHelper.h"

#ifdef CONCURRENT_ACCESS_DETECTION
TEST(ConcurrentAccessDetector, GoodCases)
{
	ConcurrentAccessDetector detectorInstance;

	{
		DETECT_CONCURRENT_ACCESS(detectorInstance);
		{
			DETECT_CONCURRENT_ACCESS(detectorInstance);
			EXPECT_NO_FATAL_FAILURE(detectorInstance.acquire());
			EXPECT_NO_FATAL_FAILURE(detectorInstance.release());
		}
	}

	{
		DETECT_CONCURRENT_ACCESS(detectorInstance);
		DETECT_CONCURRENT_ACCESS(detectorInstance);
		EXPECT_NO_FATAL_FAILURE(detectorInstance.acquire());
		EXPECT_NO_FATAL_FAILURE(detectorInstance.release());
	}
}

TEST(ConcurrentAccessDetector, BadCase)
{
	DisableFailOnAssert();

	int assertCount = 0;
	gGlobalAssertHandler = [&assertCount](){ ++assertCount; };

	{
		ConcurrentAccessDetector detectorInstance;
		{
			DETECT_CONCURRENT_ACCESS(detectorInstance);

			auto thread = std::thread([&detectorInstance]()
			{
				DETECT_CONCURRENT_ACCESS(detectorInstance);
			});
			thread.join();
		}
	}
	EXPECT_LT(0, assertCount);

	EnableFailOnAssert();
}
#endif
