#include "Base/precomp.h"

#include "Utils/Multithreading/ThreadPool.h"

#include <array>
#include <thread>
#include <unordered_map>

#include <cameron314/concurrentqueue/concurrentqueue.h>

#include "Base/Debug/Assert.h"

namespace ThreadPoolInternal
{
	struct Finalizer
	{
		using FinalizeFn = std::function<void(std::any&&)>;

		Finalizer() = default;

		template<typename Fn, typename Any>
		Finalizer(Fn&& fn, Any&& result)
			: fn(std::forward<Fn>(fn))
			, result(std::forward<Any>(result))
		{}

		FinalizeFn fn;
		std::any result;
	};

	struct FinalizerGroup
	{
		std::atomic<int> tasksNotFinalizedCount = 0;
		moodycamel::ConcurrentQueue<Finalizer> readyFinalizers;
	};

	static void FinalizeReadyTasks(std::array<Finalizer, 24>& finalizersToExecute, const size_t size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			Finalizer& finalizer = finalizersToExecute[i];
			if (finalizer.fn)
			{
				finalizer.fn(std::move(finalizer.result));
			}
		}
	}
}

struct ThreadPool::Impl
{
	std::unordered_map<size_t, std::unique_ptr<ThreadPoolInternal::FinalizerGroup>> finalizers;
	std::vector<std::thread> threads;
};

ThreadPool::ThreadPool(const size_t threadsCount, std::function<void()>&& threadPreShutdownTask)
	: mPimpl(std::make_unique<Impl>())
	, mThreadPreShutdownTask(std::move(threadPreShutdownTask))
{
	spawnThreads(threadsCount);
}

ThreadPool::~ThreadPool()
{
	shutdown();
}

void ThreadPool::shutdown()
{
	{
		std::lock_guard l(mDataMutex);
		mReadyToShutdown = true;
	}
	mWorkerThreadWakeUp.notify_all();

	for (auto& thread : mPimpl->threads)
	{
		thread.join();
	}
	mPimpl->threads.clear();
}

void ThreadPool::spawnThreads(const size_t threadsCount, const size_t firstThreadIndex)
{
	for (size_t i = 0; i < threadsCount; ++i)
	{
		const size_t threadId = mPimpl->threads.size() + firstThreadIndex;
		mPimpl->threads.emplace_back([this, threadId]
		{
			ThisThreadId = threadId;
			workerThreadFunction();
		});
	}
}

void ThreadPool::finalizeTasks(const size_t groupId)
{
	using namespace ThreadPoolInternal;

	AssertFatal(!mPimpl->threads.empty(), "No threads available to run the tasks, run threads or use processAndFinalizeTasks instead of finalizeTasks");

	std::unique_lock lock(mDataMutex);
	FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(groupId);
	lock.unlock();

	finalizeTaskForGroup(finalizerGroup);
}

void ThreadPool::processAndFinalizeTasks(const size_t finalizationGroupId)
{
	using namespace ThreadPoolInternal;

	std::array<Finalizer, 24> finalizersToExecute;

	std::unique_lock lock(mDataMutex);
	FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(finalizationGroupId);
	lock.unlock();

	while(finalizerGroup.tasksNotFinalizedCount.load(std::memory_order::acquire) > 0)
	{
		if (const size_t count = finalizerGroup.readyFinalizers.try_dequeue_bulk(finalizersToExecute.begin(), finalizersToExecute.size()); count > 0)
		{
			finalizerGroup.tasksNotFinalizedCount -= static_cast<int>(count);
			FinalizeReadyTasks(finalizersToExecute, count);
			continue;
		}

		lock.lock();
		if (!mTasksQueue.empty())
		{
			Task currentTask = std::move(mTasksQueue.front());
			mTasksQueue.pop_front();
			FinalizerGroup& taskFinalizerGroup = getOrCreateFinalizerGroup(currentTask.groupId);
			lock.unlock();

			processAndFinalizeOneTask(finalizationGroupId, taskFinalizerGroup, currentTask);
		}
		else
		{
			lock.unlock();
		}
	}
}

size_t ThreadPool::getThreadsCount() const
{
	return mPimpl->threads.size();
}

void ThreadPool::workerThreadFunction()
{
	using namespace ThreadPoolInternal;

	Task currentTask;

	while(true)
	{
		{
			std::unique_lock lock(mDataMutex);
			mWorkerThreadWakeUp.wait(lock, [&]{ return mReadyToShutdown || !mTasksQueue.empty(); });

			if (mReadyToShutdown)
			{
				break;
			}

			currentTask = std::move(mTasksQueue.front());
			mTasksQueue.pop_front();
		}

		std::any result = currentTask.taskFn();

		std::unique_lock lock(mDataMutex);
		FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(currentTask.groupId);
		lock.unlock();

		taskPostProcess(currentTask, finalizerGroup, std::move(result));
	}

	if (mThreadPreShutdownTask)
	{
		mThreadPreShutdownTask();
	}
}

void ThreadPool::taskPostProcess(Task& currentTask, ThreadPoolInternal::FinalizerGroup& finalizerGroup, std::any&& result)
{
	if (currentTask.finalizeFn)
	{
		finalizerGroup.readyFinalizers.enqueue_emplace(std::move(currentTask.finalizeFn), std::move(result));
	}
	else
	{
		[[maybe_unused]] int tasksLeftCount = 0;
		tasksLeftCount = --finalizerGroup.tasksNotFinalizedCount;
		Assert(tasksLeftCount >= 0, "finalizerGroup.tasksLeftCount should never be negative");
	}
}

void ThreadPool::finalizeTaskForGroup(ThreadPoolInternal::FinalizerGroup& finalizerGroup)
{
	using namespace ThreadPoolInternal;

	std::array<Finalizer, 24> finalizersToExecute;

	while(finalizerGroup.tasksNotFinalizedCount.load(std::memory_order::acquire) > 0)
	{
		if (const size_t count = finalizerGroup.readyFinalizers.try_dequeue_bulk(finalizersToExecute.begin(), finalizersToExecute.size()); count > 0)
		{
			finalizerGroup.tasksNotFinalizedCount -= static_cast<int>(count);
			FinalizeReadyTasks(finalizersToExecute, count);
		}
	}
}

void ThreadPool::processAndFinalizeOneTask(const size_t finalizationGroupId, ThreadPoolInternal::FinalizerGroup& finalizerGroup, Task& currentTask)
{
	std::any result = currentTask.taskFn();

	if (currentTask.groupId == finalizationGroupId)
	{
		if (currentTask.finalizeFn)
		{
			currentTask.finalizeFn(std::move(result));
		}
		--finalizerGroup.tasksNotFinalizedCount;
	}
	else
	{
		taskPostProcess(currentTask, finalizerGroup, std::move(result));
	}
}

ThreadPoolInternal::FinalizerGroup& ThreadPool::getOrCreateFinalizerGroup(const size_t groupId)
{
	using namespace ThreadPoolInternal;

	std::unique_ptr<FinalizerGroup>& groupPtr = mPimpl->finalizers[groupId];

	if (!groupPtr)
	{
		groupPtr = std::make_unique<FinalizerGroup>();
	}
	return *groupPtr;
}

void ThreadPool::incrementNotFinalizedTasksCount(const size_t groupId, const size_t addedCount)
{
	using namespace ThreadPoolInternal;

	FinalizerGroup& group = getOrCreateFinalizerGroup(groupId);
	group.tasksNotFinalizedCount += static_cast<int>(addedCount);
}
