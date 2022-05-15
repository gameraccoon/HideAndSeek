#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "UnitTests/TestAssertHelper.h"

#ifdef CONCURRENT_ACCESS_DETECTION
TEST(ConcurrentAccessDetector, SingleThreadedAccess)
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

	{
		DETECT_CONCURRENT_ACCESS_UNLOCKABLE(detectorInstance, dataRaceDetector);
		CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(dataRaceDetector);
		CONCURRENT_ACCESS_DETECTOR_MANUAL_LOCK(dataRaceDetector);
	}
}

TEST(ConcurrentAccessDetector, AccessedFromMultipleThreads)
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
	EXPECT_GT(assertCount, 0);

	EnableFailOnAssert();
}

TEST(ConcurrentAccessDetector, UnlockableGuard_UnlockedWhenAccessedFromMultipleThreads)
{
	DisableFailOnAssert();

	int assertCount = 0;
	gGlobalAssertHandler = [&assertCount](){ ++assertCount; };

	{
		ConcurrentAccessDetector detectorInstance;
		{
			DETECT_CONCURRENT_ACCESS_UNLOCKABLE(detectorInstance, detectorMain);

			CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(detectorMain);
			auto thread = std::thread([&detectorInstance]()
			{
				DETECT_CONCURRENT_ACCESS(detectorInstance);
			});
			thread.join();
			CONCURRENT_ACCESS_DETECTOR_MANUAL_LOCK(detectorMain);
		}
	}
	EXPECT_EQ(assertCount, 0);

	EnableFailOnAssert();
}

TEST(ConcurrentAccessDetector, UnlockableGuard_AccessedFromMultipleThreads)
{
	DisableFailOnAssert();

	int assertCount = 0;
	gGlobalAssertHandler = [&assertCount](){ ++assertCount; };

	{
		ConcurrentAccessDetector detectorInstance;
		{
			DETECT_CONCURRENT_ACCESS_UNLOCKABLE(detectorInstance, detectorMain);

			auto thread = std::thread([&detectorInstance]()
			{
				DETECT_CONCURRENT_ACCESS(detectorInstance);
			});
			thread.join();

			CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(detectorMain);
			CONCURRENT_ACCESS_DETECTOR_MANUAL_LOCK(detectorMain);

			auto thread2 = std::thread([&detectorInstance]()
			{
				DETECT_CONCURRENT_ACCESS(detectorInstance);
			});
			thread2.join();
			CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(detectorMain);
		}
	}
	EXPECT_GE(assertCount, 2);

	EnableFailOnAssert();
}
#endif
