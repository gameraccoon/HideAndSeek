#pragma once

#include <any>
#include <condition_variable>
#include <functional>
#include <list>
#include <thread>
#include <vector>

#include <cameron314/concurrentqueue/concurrentqueue.h>

class ThreadPool
{
public:
	template<typename Func = std::nullptr_t>
	ThreadPool(size_t threadsCount = 0, Func&& threadPreShutdownTask = nullptr)
		: mThreadPreShutdownTask(std::forward<Func>(threadPreShutdownTask))
	{
		spawnThreads(threadsCount);
	}

	~ThreadPool()
	{
		shutdown();
	}

	void shutdown()
	{
		{
			std::lock_guard l(mDataMutex);
			mReadyToShutdown = true;
		}
		mWorkerThreadWakeUp.notify_all();

		for (auto& thread : mThreads)
		{
			thread.join();
		}
		mThreads.clear();
	}

	void spawnThreads(size_t threadsCount, size_t firstThreadIndex = 1)
	{
		for (size_t i = 0; i < threadsCount; ++i)
		{
			const size_t threadId = mThreads.size() + firstThreadIndex;
			mThreads.emplace_back([this, threadId]
			{
				ThisThreadId = threadId;
				workerThreadFunction();
			});
		}
	}

	/**
	 * @brief executeTask
	 * @param taskFn
	 * @param finalizeFn
	 * @param groupId  an unique group id to be able to separate task finalization for different groups
	 *
	 * Can be safely called during a finalizeFn of another task
	 */
	template<typename TaskFnT, typename FinalizeFnT>
	void executeTask(TaskFnT&& taskFn, FinalizeFnT&& finalizeFn, size_t groupId = 0, bool wakeUpThread = true)
	{
		AssertFatal(!mThreads.empty(), "No threads to execute the task");

		{
			std::lock_guard<std::mutex> lock(mDataMutex);
			FinalizerGroup& group = getOrCreateFinalizerGroup(groupId);
			++group.tasksNotFinalizedCount;

			mTasksQueue.emplace_back(groupId, std::forward<TaskFnT>(taskFn), std::forward<FinalizeFnT>(finalizeFn));
		}

		if (wakeUpThread)
		{
			mWorkerThreadWakeUp.notify_one();
		}
	}

	template<typename TaskFnT, typename FinalizeFnT>
	void executeTasks(std::vector<std::pair<TaskFnT, FinalizeFnT>>&& tasks, size_t groupId = 0, size_t threadsToWakeUp = std::numeric_limits<size_t>::max())
	{
		AssertFatal(!mThreads.empty(), "No threads to execute the task");

		threadsToWakeUp = std::max(tasks.size(), threadsToWakeUp);

		{
			std::lock_guard l(mDataMutex);

			FinalizerGroup& group = getOrCreateFinalizerGroup(groupId);

			group.tasksNotFinalizedCount += static_cast<int>(tasks.size());

			for (auto& task : tasks)
			{
				mTasksQueue.emplace_back(groupId, std::move(task.first), std::move(task.second));
			}
		}

		if (threadsToWakeUp >= mThreads.size())
		{
			mWorkerThreadWakeUp.notify_all();
		}
		else
		{
			for (size_t i = 0; i < threadsToWakeUp; ++i)
			{
				mWorkerThreadWakeUp.notify_one();
			}
		}
	}

	/**
	 * @brief finalizeTasks
	 * @param groupId
	 */
	void finalizeTasks(size_t groupId = 0)
	{
		std::unique_lock lock(mDataMutex);
		FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(groupId);
		lock.unlock();

		finalizeTaskForGroup(finalizerGroup);
	}

	void processAndFinalizeTasks(size_t groupId = 0)
	{
		std::vector<Finalizer> finalizersToExecute(24);
		Task currentTask;

		std::unique_lock lock(mDataMutex);
		FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(groupId);
		lock.unlock();

		while(finalizerGroup.tasksNotFinalizedCount.load(std::memory_order::acquire) > 0)
		{
			if (size_t count = finalizerGroup.readyFinalizers.try_dequeue_bulk(finalizersToExecute.begin(), finalizersToExecute.size()); count > 0)
			{
				finalizerGroup.tasksNotFinalizedCount -= count;
				finalizeReadyTasks(finalizersToExecute, count);
				continue;
			}

			lock.lock();
			if (!mTasksQueue.empty())
			{
				currentTask = std::move(mTasksQueue.front());
				mTasksQueue.pop_front();
				FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(currentTask.groupId);
				lock.unlock();

				processAndFinalizeOneTask(groupId, finalizerGroup, currentTask);
			}
			else
			{
				lock.unlock();
			}
		}
	}

	static size_t GetThisThreadId() { return ThisThreadId; }

	/**
	 * Safe to call if we do it after all spawnThreads calls
	 */
	size_t getThreadsCount() const { return mThreads.size(); }

private:
	using TaskFn = std::function<std::any()>;
	using FinalizeFn = std::function<void(std::any&&)>;

	struct Finalizer
	{
		FinalizeFn fn;
		std::any result;
	};

	struct FinalizerGroup
	{
		std::atomic<int> tasksNotFinalizedCount = 0;
		moodycamel::ConcurrentQueue<Finalizer> readyFinalizers;
	};

	struct Task
	{
		Task() = default;

		template<typename TaskFnT, typename FinalizeFnT>
		Task(size_t groupId, TaskFnT&& taskFn, FinalizeFnT&& finalizeFn = nullptr)
			: groupId(groupId)
			, taskFn(std::move(taskFn))
			, finalizeFn(std::move(finalizeFn))
		{}

		size_t groupId;
		TaskFn taskFn;
		FinalizeFn finalizeFn;
	};

	void workerThreadFunction()
	{
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

			taskPostProcess(currentTask, std::move(result));
		}

		if (mThreadPreShutdownTask)
		{
			mThreadPreShutdownTask();
		}
	}

	void taskPostProcess(Task& currentTask, std::any&& result)
	{
		std::unique_lock lock(mDataMutex);
		FinalizerGroup& finalizerGroup = getOrCreateFinalizerGroup(currentTask.groupId);
		lock.unlock();

		if (currentTask.finalizeFn)
		{
			finalizerGroup.readyFinalizers.enqueue_emplace(std::move(currentTask.finalizeFn), std::move(result));
		}
		else
		{
			size_t tasksLeftCount = 0;
			tasksLeftCount = --finalizerGroup.tasksNotFinalizedCount;

			if (tasksLeftCount <= 0)
			{
				Assert(tasksLeftCount == 0, "finalizerGroup.tasksLeftCount should never be negative");
			}
		}
	}

	void finalizeTaskForGroup(FinalizerGroup& finalizerGroup)
	{
		std::vector<Finalizer> finalizersToExecute(24);

		while(finalizerGroup.tasksNotFinalizedCount.load(std::memory_order::acquire) > 0)
		{
			if (size_t count = finalizerGroup.readyFinalizers.try_dequeue_bulk(finalizersToExecute.begin(), finalizersToExecute.size()); count > 0)
			{
				finalizerGroup.tasksNotFinalizedCount -= count;
				finalizeReadyTasks(finalizersToExecute, count);
			}
		}
	}

	void finalizeReadyTasks(std::vector<Finalizer>& finalizersToExecute, size_t size)
	{
		if (!finalizersToExecute.empty())
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

	FinalizerGroup& getOrCreateFinalizerGroup(size_t groupId)
	{
		std::unique_ptr<FinalizerGroup>& groupPtr = mFinalizers[groupId];

		if (!groupPtr)
		{
			groupPtr = std::make_unique<FinalizerGroup>();
		}
		return *groupPtr;
	}

	void processAndFinalizeOneTask(size_t groupId, FinalizerGroup& finalizerGroup, Task& currentTask)
	{
		std::any result = currentTask.taskFn();

		if (currentTask.groupId == groupId)
		{
			if (currentTask.finalizeFn)
			{
				currentTask.finalizeFn(std::move(result));
			}
			--finalizerGroup.tasksNotFinalizedCount;
		}
	}

private:
	std::mutex mDataMutex;
	bool mReadyToShutdown = false;
	std::list<Task> mTasksQueue;
	std::unordered_map<size_t, std::unique_ptr<FinalizerGroup>> mFinalizers;

	std::vector<std::thread> mThreads;
	const std::function<void()> mThreadPreShutdownTask;

	std::condition_variable mWorkerThreadWakeUp;

	static inline thread_local size_t ThisThreadId = 0;
};
