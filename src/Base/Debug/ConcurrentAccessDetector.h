#pragma once

#include <atomic>
#include <thread>

#include "Base/Debug/Assert.h"

#ifdef CONCURRENT_ACCESS_DETECTION
/**
 * You can use this object to detect data races in debug mode, skipping any checks in release builds
 * Use it as if you use mutex:
 * a) create an object of ConcurrentAccessDetector
 * b) acquire it at the beginning of the potential critical section
 * c) release it at the end of the potential critical section
 *
 * You can also use ConcurrentAccessDetector::Guard(instance); to serve as std::lock_guard for a mutex
 */
class ConcurrentAccessDetector
{
public:
	class Guard
	{
	public:
		explicit Guard(ConcurrentAccessDetector& detector)
			: mDetector(detector)
		{
            mDetector.acquire();
		}

		~Guard()
		{
			mDetector.release();
		}

		Guard(const Guard&) = delete;
		Guard& operator==(const Guard&) = delete;
		Guard(Guard&&) = delete;
		Guard& operator==(Guard&&) = delete;

	private:
		ConcurrentAccessDetector& mDetector;
	};

public:
	ConcurrentAccessDetector() = default;
	~ConcurrentAccessDetector()
	{
		Assert(mAcquiredCount.load(std::memory_order_relaxed) == 0, "Not all threads released ConcurrentAccessDetector or a data race occurred");
	}

	ConcurrentAccessDetector(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector& operator=(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector(ConcurrentAccessDetector&&) = delete;
	ConcurrentAccessDetector& operator=(ConcurrentAccessDetector&&) = delete;

	void acquire()
	{
		// Note that this code doesn't have to be 100% thread-safe
		// covering 90% cases is enough to detect data races
		const int acquiredCountBefore = mAcquiredCount.fetch_add(1, std::memory_order_relaxed);
		if (acquiredCountBefore > 0)
		{
			std::thread::id ownedThreadID = mOwningThreadID.load(std::memory_order_relaxed);
			std::thread::id currentThreadID = std::this_thread::get_id();
			if (ownedThreadID != currentThreadID)
			{
				ReportErrorRelease("A data race detected");
			}
		}
		else
		{
			mOwningThreadID.store(std::this_thread::get_id(), std::memory_order_relaxed);
		}
	}

	void release()
	{
		mAcquiredCount.fetch_sub(1, std::memory_order_relaxed);
	}

private:
	std::atomic<int> mAcquiredCount{0};
	std::atomic<std::thread::id> mOwningThreadID;
};
#else
class ConcurrentAccessDetector
{
public:
	class Guard
	{
	public:
		explicit Guard(ConcurrentAccessDetector&) {}
		~Guard() = default;
		Guard(const Guard&) = delete;
		Guard& operator=(const Guard&) = delete;
		Guard(Guard&&) = delete;
		Guard& operator=(Guard&&) = delete;
	};
public:
	ConcurrentAccessDetector() = default;
	~ConcurrentAccessDetector() = default;
	ConcurrentAccessDetector(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector& operator=(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector(ConcurrentAccessDetector&&) = delete;
	ConcurrentAccessDetector& operator=(ConcurrentAccessDetector&&) = delete;

	void aquire() {}
	void release() {}
};
#endif // CONCURRENT_ACCESS_DETECTION

#ifdef CONCURRENT_ACCESS_DETECTION
#define DETECT_CONCURRENT_ACCESS_NAME(A,B) A##B
#define DETECT_CONCURRENT_ACCESS_IMPL(dataRaceDetector, namePostfix) ConcurrentAccessDetector::Guard DETECT_CONCURRENT_ACCESS_NAME(cadg_inst_, namePostfix)(dataRaceDetector)
// macro generates a unique instance name of the guard for us
#define DETECT_CONCURRENT_ACCESS(dataRaceDetector) DETECT_CONCURRENT_ACCESS_IMPL(dataRaceDetector, __COUNTER__)
#else
#define DETECT_CONCURRENT_ACCESS(dataRaceDetector)
#endif // CONCURRENT_ACCESS_DETECTION
