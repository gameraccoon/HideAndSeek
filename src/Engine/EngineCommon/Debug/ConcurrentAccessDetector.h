#pragma once

#ifdef CONCURRENT_ACCESS_DETECTION

#include <atomic>
#include <thread>

#include "EngineCommon/Debug/Assert.h"

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
		template<typename Func = std::nullptr_t>
		explicit Guard(ConcurrentAccessDetector& detector, const char* filename = "", int line = 0, Func errorHandler = nullptr)
			: mDetector(detector)
		{
			mDetector.acquire(filename, line, errorHandler);
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

	class UniqueGuard
	{
	public:
		template<typename Func = std::nullptr_t>
		explicit UniqueGuard(ConcurrentAccessDetector& detector, const char* filename = "", int line = 0, Func errorHandler = nullptr)
			: mDetector(detector)
		{
			mDetector.acquire(filename, line, errorHandler);
		}

		~UniqueGuard()
		{
			if (mIsAcquired)
			{
				mDetector.release();
			}
		}

		void release()
		{
			AssertFatal(mIsAcquired, "Tried to release non-acquired UniqueGuard");
			mDetector.release();
			mIsAcquired = false;
		}

		template<typename Func = std::nullptr_t>
		void reacquire(const char* filename = "", int line = 0, Func errorHandler = nullptr)
		{
			AssertFatal(!mIsAcquired, "Tried to reacquire non-acquired UniqueGuard");
			mDetector.acquire(filename, line, errorHandler);
			mIsAcquired = true;
		}

		UniqueGuard(const Guard&) = delete;
		UniqueGuard& operator==(const Guard&) = delete;
		UniqueGuard(Guard&&) = delete;
		UniqueGuard& operator==(Guard&&) = delete;

	private:
		ConcurrentAccessDetector& mDetector;
		bool mIsAcquired = true;
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

	template<typename Func = std::nullptr_t>
	void acquire(const char* filename = "", int line = 0, Func errorHandler = nullptr)
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
				if constexpr (!std::is_same<Func, std::nullptr_t>::value)
				{
					errorHandler();
				}
				else
				{
					ReportErrorRelease("A data race detected (%s:%d) and (%s:%d)", mLastLockedFile, mLastLockedLine, filename, line);
					(void)errorHandler;
				}
			}
		}
		else
		{
			mOwningThreadID.store(std::this_thread::get_id(), std::memory_order_relaxed);
			mLastLockedFile = filename;
			mLastLockedLine = line;
		}
	}

	void release()
	{
		mAcquiredCount.fetch_sub(1, std::memory_order_relaxed);
	}

private:
	std::atomic<int> mAcquiredCount{0};
	std::atomic<std::thread::id> mOwningThreadID;
	const char* mLastLockedFile = "";
	int mLastLockedLine = 0;
};
#else
class ConcurrentAccessDetector
{
public:
	ConcurrentAccessDetector() = default;
	~ConcurrentAccessDetector() = default;
	ConcurrentAccessDetector(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector& operator=(const ConcurrentAccessDetector&) = delete;
	ConcurrentAccessDetector(ConcurrentAccessDetector&&) = delete;
	ConcurrentAccessDetector& operator=(ConcurrentAccessDetector&&) = delete;
};
#endif // CONCURRENT_ACCESS_DETECTION

#ifdef CONCURRENT_ACCESS_DETECTION

#define HELPER_DETECT_CONCURRENT_ACCESS_NAME(A,B) A##B
#define HELPER_DETECT_CONCURRENT_ACCESS_IMPL(dataRaceDetector, namePostfix) ConcurrentAccessDetector::Guard HELPER_DETECT_CONCURRENT_ACCESS_NAME(cadg_inst_, namePostfix)((dataRaceDetector), __FILE__, __LINE__)
// macro generates a unique instance name of the guard for us
#define DETECT_CONCURRENT_ACCESS(dataRaceDetector) HELPER_DETECT_CONCURRENT_ACCESS_IMPL((dataRaceDetector), __COUNTER__)

// we specify the name of the guard as the second argument and should use it to unlock or lock back the guard
#define DETECT_CONCURRENT_ACCESS_UNLOCKABLE(dataRaceDetector, guardName) ConcurrentAccessDetector::UniqueGuard (guardName)((dataRaceDetector), __FILE__, __LINE__)
#define CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(guard) guard.release();
#define CONCURRENT_ACCESS_DETECTOR_MANUAL_LOCK(guard) guard.reacquire(__FILE__, __LINE__);

#else

#define DETECT_CONCURRENT_ACCESS(dataRaceDetector)
#define DETECT_CONCURRENT_ACCESS_UNLOCKABLE(dataRaceDetector, guardName)
#define CONCURRENT_ACCESS_DETECTOR_MANUAL_UNLOCK(guard)
#define CONCURRENT_ACCESS_DETECTOR_MANUAL_LOCK(guard)

#endif // CONCURRENT_ACCESS_DETECTION
