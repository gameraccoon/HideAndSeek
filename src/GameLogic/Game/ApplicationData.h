#pragma once

#include "HAL/Base/Engine.h"

#include "Utils/Multithreading/ThreadPool.h"
#ifdef ENABLE_SCOPED_PROFILER
#include "Utils/Profiling/ProfileDataWriter.h"
#endif // ENABLE_SCOPED_PROFILER

#include "GameLogic/Render/RenderThreadManager.h"

class ApplicationData
{
public:
	static constexpr int DefaultWorkerThreadCount = 3;
	const int MainThreadId = 0;
	const int WorkerThreadsCount = DefaultWorkerThreadCount;
	const int RenderThreadId = WorkerThreadsCount + 1;
	std::string ScopedProfileOutputPath = "./scoped_profile.json";

	ThreadPool threadPool;
	RenderThreadManager renderThread;

public:
	ApplicationData(int threadsCount)
		: WorkerThreadsCount(threadsCount)
		, RenderThreadId(threadsCount + 1)
		, threadPool(threadsCount, [this]{ workingThreadSaveProfileData(); })
	{
	}

	void writeProfilingData()
	{
#ifdef ENABLE_SCOPED_PROFILER
		{
			ProfileDataWriter::ProfileData data;
			{
				data.scopedProfilerDatas.emplace_back();
				ProfileDataWriter::ScopedProfilerData& renderScopedProfilerData = data.scopedProfilerDatas.back();
				renderScopedProfilerData.records = renderThread.getAccessor().consumeScopedProfilerRecordsUnsafe();
				renderScopedProfilerData.threadId = RenderThreadId;
			}
			{
				data.scopedProfilerDatas.emplace_back();
				ProfileDataWriter::ScopedProfilerData& mainScopedProfilerData = data.scopedProfilerDatas.back();
				mainScopedProfilerData.records = gtlScopedProfilerData.getAllRecords();
				mainScopedProfilerData.threadId = MainThreadId;
			}

			for (auto&& [threadId, records] : mScopedProfileRecords)
			{
				data.scopedProfilerDatas.emplace_back(threadId, std::move(records));
			}

			data.threadNames.resize(RenderThreadId + 1);
			data.threadNames[MainThreadId] = "Main Thread";
			data.threadNames[RenderThreadId] = "Render Thread";
			for (int i = 0; i < WorkerThreadsCount; ++i)
			{
				// zero is reserved for main thread
				data.threadNames[1 + i] = std::string("Worker Thread #") + std::to_string(i+1);
			}

			ProfileDataWriter::PrintScopedProfileToFile(ScopedProfileOutputPath, data);
		}
#endif // ENABLE_SCOPED_PROFILER
	}

	void workingThreadSaveProfileData()
	{
#ifdef ENABLE_SCOPED_PROFILER
		std::lock_guard l(mScopedProfileRecordsMutex);
		mScopedProfileRecords.emplace_back(ThreadPool::GetThisThreadId(), gtlScopedProfilerData.getAllRecords());
#endif // ENABLE_SCOPED_PROFILER
	}

	void shutdownThreads()
	{
		threadPool.shutdown();
		renderThread.shutdownThread();
	}

private:
	std::vector<std::pair<size_t, ScopedProfilerThreadData::Records>> mScopedProfileRecords;
	std::mutex mScopedProfileRecordsMutex;
};
