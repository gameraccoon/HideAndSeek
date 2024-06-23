#include "Base/precomp.h"

#include <gtest/gtest.h>

#include <thread>

#include "Utils/Multithreading/ThreadPool.h"

TEST(ThreadPool, CreateAndDestroyEmpty)
{
	ThreadPool threadPool;
}

TEST(ThreadPool, CreateAndDestroyWithThreads)
{
	{
		ThreadPool threadPool{2};
	}

	{
		ThreadPool threadPool;
		threadPool.spawnThreads(2);
	}
}

TEST(ThreadPool, ThreadPreShutdownTask)
{
	std::atomic<int> callCount = 0;
	{
		ThreadPool threadPool{2, [&callCount]{ ++callCount; }};
		EXPECT_EQ(callCount, 0);
		threadPool.shutdown();
		EXPECT_EQ(callCount, 2);
	}

	{
		EXPECT_EQ(callCount, 2);
		ThreadPool threadPool{ 2, [&callCount]{ ++callCount; } };
		EXPECT_EQ(callCount, 2);
	}
	EXPECT_EQ(callCount, 4);
}

TEST(ThreadPool, ExecuteOneTaskWithoutFinalizer)
{
	std::atomic<int> callCount = 0;

	ThreadPool threadPool{2};
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, nullptr);
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 1);
}

TEST(ThreadPool, ExecuteOneTaskWithFinalizer)
{
	std::atomic<int> callCount = 0;
	int finalizeCount = 0;

	ThreadPool threadPool{2};
	threadPool.executeTask([&callCount]{ ++callCount; return 3; }, [&finalizeCount](std::any&& val) { finalizeCount += std::any_cast<int>(val); });
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 1);
	EXPECT_EQ(finalizeCount, 3);
}

TEST(ThreadPool, ExecuteOneTaskWithFinalizer_CheckThreads)
{
	auto mainThreadId = std::this_thread::get_id();

	ThreadPool threadPool{2};
	threadPool.executeTask(
		[&mainThreadId]
		{
			// if we call "finalizeTasks" we never use main thread to execute tasks
			EXPECT_NE(std::this_thread::get_id(), mainThreadId);
			return std::any();
		},
		[&mainThreadId](std::any&&)
		{
			EXPECT_EQ(std::this_thread::get_id(), mainThreadId);
		}
	);
	threadPool.finalizeTasks();
}

TEST(ThreadPool, ExecuteTwoTasksWithoutFinalizer)
{
	std::atomic<int> callCount = 0;

	ThreadPool threadPool{2};
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, nullptr);
	threadPool.executeTask([&callCount]{ callCount += 5; return std::any(); }, nullptr);
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 6);
}

TEST(ThreadPool, ExecuteTwoTasksWithFinalizer)
{
	std::atomic<int> callCount = 0;
	int finalizeCount = 0;

	ThreadPool threadPool{2};
	threadPool.executeTask([&callCount]{ ++callCount; return 3; }, [&finalizeCount](std::any && val){ finalizeCount += std::any_cast<int>(val); });
	threadPool.executeTask([&callCount]{ callCount += 5;  return 4; }, [&finalizeCount](std::any && val){ finalizeCount += std::any_cast<int>(val) / 2; });
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 6);
	EXPECT_EQ(finalizeCount, 5);
}

TEST(ThreadPool, ExecuteTaskGroupWithoutFinalizer)
{
	std::atomic<int> callCount = 0;

	ThreadPool threadPool{2};
	const auto task = std::make_pair([&callCount]{ ++callCount; return nullptr; }, std::function<void(std::any&&)>{});
	std::vector tasks(20, task);

	threadPool.executeTasks(std::move(tasks));
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 20);
}

TEST(ThreadPool, ExecuteTaskGroupWithFinalizer)
{
	std::atomic<int> callCount = 0;
	int finalizeCount = 0;

	ThreadPool threadPool{ 2 };
	const auto task = std::make_pair([&callCount]{ ++callCount; return 1; }, [&finalizeCount](std::any&& val){ finalizeCount += std::any_cast<int>(val); });
	std::vector tasks(20, task);
	threadPool.executeTasks(std::move(tasks));
	threadPool.finalizeTasks();

	EXPECT_EQ(callCount, 20);
	EXPECT_EQ(finalizeCount, 20);
}

TEST(ThreadPool, ExecuteTaskGroupIncludingMainThread)
{
	std::atomic<int> callCount = 0;
	int finalizeCount = 0;

	ThreadPool threadPool{2};
	const auto task = std::make_pair([&callCount]{ ++callCount; return 1; }, [&finalizeCount](std::any && val){ finalizeCount += std::any_cast<int>(val); });
	std::vector tasks(20, task);
	threadPool.executeTasks(std::move(tasks));
	threadPool.processAndFinalizeTasks();

	EXPECT_EQ(callCount, 20);
	EXPECT_EQ(finalizeCount, 20);
}

TEST(ThreadPool, ExecuteTaskGroupOnlyInMainThread)
{
	std::atomic<int> callCount = 0;
	int finalizeCount = 0;

	ThreadPool threadPool{};
	const auto task = std::make_pair([&callCount]{ ++callCount; return 1; }, [&finalizeCount](std::any && val){ finalizeCount += std::any_cast<int>(val); });
	std::vector tasks(20, task);
	threadPool.executeTasks(std::move(tasks));
	threadPool.processAndFinalizeTasks();

	EXPECT_EQ(callCount, 20);
	EXPECT_EQ(finalizeCount, 20);
}

TEST(ThreadPool, CreateNewTasksInTasks)
{
	std::atomic<int> callCount = 0;
	std::atomic<int> taskCountdown = 5;

	ThreadPool threadPool{2};
	const auto task = std::make_pair
	(
		[&callCount, &taskCountdown, &threadPool]
		{
			++callCount;
			const int value = --taskCountdown;
			if (value >= 0)
			{
				threadPool.executeTask([&callCount, value]{ callCount += value; return std::any(); }, nullptr);
			}
			return std::any();
		},
		std::function<void(std::any &&)>{}
	);

	std::vector tasks(20, task);
	threadPool.executeTasks(std::move(tasks));
	threadPool.processAndFinalizeTasks();

	EXPECT_EQ(callCount, 30);
}

TEST(ThreadPool, CreateNewTasksInFinalizers)
{
	std::atomic<int> callCount = 0;
	std::atomic<int> taskCountdown = 5;

	ThreadPool threadPool{2};
	const auto task = std::make_pair
	(
		[&callCount, &taskCountdown]
		{
			++callCount;
			const int countdown = --taskCountdown;
			return countdown;
		},
		[&callCount, &threadPool](std::any && val)
		{
			const int value = std::any_cast<int>(val);
			if (value >= 0)
			{
				threadPool.executeTask([value, &callCount]{ callCount += value; return std::any(); }, nullptr);
			}
		}
	);

	std::vector tasks(20, task);
	threadPool.executeTasks(std::move(tasks));
	threadPool.processAndFinalizeTasks();

	EXPECT_EQ(callCount, 30);
}

TEST(ThreadPool, ProcessTwoGroupsInParallel)
{
	std::atomic<int> callCount = 0;
	int finalizeCount1 = 0;
	int finalizeCount2 = 0;

	ThreadPool threadPool{2};

	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, [&finalizeCount1](std::any&&){ ++finalizeCount1; }, 0);
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, [&finalizeCount2](std::any&&){ ++finalizeCount2; }, 1);
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, [&finalizeCount1](std::any&&){ ++finalizeCount1; }, 0);
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, [&finalizeCount2](std::any&&){ ++finalizeCount2; }, 1);
	threadPool.executeTask([&callCount]{ ++callCount; return std::any(); }, [&finalizeCount1](std::any&&){ ++finalizeCount1; }, 0);

	threadPool.processAndFinalizeTasks(0);
	std::thread secondGroupProcessor([&threadPool]
	{
		threadPool.processAndFinalizeTasks(1);
	});

	secondGroupProcessor.join();

	EXPECT_EQ(callCount, 5);
	EXPECT_EQ(finalizeCount1, 3);
	EXPECT_EQ(finalizeCount2, 2);
}
