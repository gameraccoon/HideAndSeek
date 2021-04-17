#pragma once

#include <vector>
#include <list>
#include <atomic>
#include <condition_variable>
#include <thread>

#include "Utils/Jobs/Job.h"

namespace Jobs
{
	class WorkerManager
	{
	public:
		explicit WorkerManager(size_t threadsCount);
		~WorkerManager();
		WorkerManager(const WorkerManager&) = delete;
		WorkerManager& operator=(const WorkerManager&) = delete;
		WorkerManager(WorkerManager&&) = delete;
		WorkerManager& operator=(WorkerManager&&) = delete;

		void runJobs(std::vector<BaseJob::UniquePtr>&& jobs);

		[[nodiscard]] size_t getThreadsCount() const { return mThreads.size(); }

	private:
		void threadFunction();
		BaseJob::JobGroupId generateNextId() { return mNextId.fetch_add(1, std::memory_order_relaxed); }

	private:
		std::vector<std::thread> mThreads;

		std::mutex mDataMutex;
		bool mNeedShutdown = false;
		// queue for working threads
		std::list<BaseJob::UniquePtr> mPlannedJobs;
		std::condition_variable mPlannedJobsTrigger;
		// queue for started threads for finalization
		std::list<BaseJob::UniquePtr> mFinishedJobs;
		std::condition_variable mFinishedJobsTrigger;

		std::atomic<BaseJob::JobGroupId> mNextId = 0;
	};

	size_t GetAvailableThreadsCount();
}
