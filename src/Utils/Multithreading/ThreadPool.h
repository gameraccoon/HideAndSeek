#pragma once

#include <any>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace ThreadPoolInternal
{
	struct FinalizerGroup;
}

class ThreadPool
{
public:
	explicit ThreadPool(size_t threadsCount = 0, std::function<void()>&& threadPreShutdownTask = nullptr);

	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;

	void shutdown();

	void spawnThreads(size_t threadsCount, size_t firstThreadIndex = 1);

	/**
	 * @brief executeTask
	 * @param taskFn
	 * @param finalizeFn
	 * @param groupId  an unique group id to be able to separate task finalization for different groups
	 * @param wakeUpThread  if true, will tru to wake up a thread to process the task, useful to set it
	 * to false if we are scheduling multiple tesks and this one is not the last one
	 *
	 * Can be safely called during a finalizeFn of another task
	 */
	template<typename TaskFnT, typename FinalizeFnT>
	void executeTask(TaskFnT&& taskFn, FinalizeFnT&& finalizeFn, size_t groupId = 0, const bool wakeUpThread = true)
	{
		{
			std::lock_guard<std::mutex> lock(mDataMutex);
			incrementNotFinalizedTasksCount(groupId, 1);

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
		threadsToWakeUp = std::max(tasks.size(), threadsToWakeUp);

		{
			std::lock_guard l(mDataMutex);

			incrementNotFinalizedTasksCount(groupId, tasks.size());

			for (auto& task : tasks)
			{
				mTasksQueue.emplace_back(groupId, std::move(task.first), std::move(task.second));
			}
		}

		if (threadsToWakeUp >= getThreadsCount())
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
	void finalizeTasks(size_t groupId = 0);

	void processAndFinalizeTasks(size_t finalizationGroupId = 0);

	static size_t GetThisThreadId() { return ThisThreadId; }

	/**
	 * Safe to call if we do it after all spawnThreads calls
	 */
	size_t getThreadsCount() const;

private:
	using TaskFn = std::function<std::any()>;
	using FinalizeFn = std::function<void(std::any&&)>;

	struct Task
	{
		Task() = default;

		template<typename TaskFnT, typename FinalizeFnT>
		Task(const size_t groupId, TaskFnT&& taskFn, FinalizeFnT&& finalizeFn = nullptr)
			: groupId(groupId)
			, taskFn(std::forward<TaskFnT>(taskFn))
			, finalizeFn(std::forward<FinalizeFnT>(finalizeFn))
		{}

		size_t groupId = 0;
		TaskFn taskFn;
		FinalizeFn finalizeFn;
	};

	void workerThreadFunction();
	static void taskPostProcess(Task& currentTask, ThreadPoolInternal::FinalizerGroup& finalizerGroup, std::any&& result);

	static void finalizeTaskForGroup(ThreadPoolInternal::FinalizerGroup& finalizerGroup);
	static void processAndFinalizeOneTask(size_t finalizationGroupId, ThreadPoolInternal::FinalizerGroup& finalizerGroup, Task& currentTask);

	ThreadPoolInternal::FinalizerGroup& getOrCreateFinalizerGroup(size_t groupId);
	void incrementNotFinalizedTasksCount(size_t groupId, size_t addedCount);

private:
	std::mutex mDataMutex;
	bool mReadyToShutdown = false;
	std::list<Task> mTasksQueue;

	struct Impl;
	std::unique_ptr<Impl> mPimpl;

	const std::function<void()> mThreadPreShutdownTask;

	std::condition_variable mWorkerThreadWakeUp;

	static inline thread_local size_t ThisThreadId = 0;
};
