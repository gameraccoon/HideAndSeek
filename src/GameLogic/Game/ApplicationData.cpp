#include "EngineCommon/precomp.h"

#include "GameLogic/Game/ApplicationData.h"

#ifdef ENABLE_SCOPED_PROFILER
#include "GameUtils/Profiling/ProfileDataWriter.h"
#endif // ENABLE_SCOPED_PROFILER

ApplicationData::ApplicationData(const int threadsCount)
	: WorkerThreadsCount(threadsCount)
	, RenderThreadId(threadsCount + 1)
	, ResourceLoadingThreadId(threadsCount + 2)
	, threadPool(threadsCount, [] { threadSaveProfileData(ThreadPool::GetThisThreadId()); })
{
	resourceManager.startLoadingThread([this] { threadSaveProfileData(ResourceLoadingThreadId); });
}

void ApplicationData::writeProfilingData()
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

		data.threadNames.resize(RenderThreadId + 2);
		data.threadNames[MainThreadId] = "Main Thread";
		data.threadNames[RenderThreadId] = "Render Thread";
		data.threadNames[ResourceLoadingThreadId] = "Resource Loading Thread";
		for (int i = 0; i < WorkerThreadsCount; ++i)
		{
			// zero is reserved for main thread
			data.threadNames[1 + i] = std::string("Worker Thread #") + std::to_string(i + 1);
		}

		ProfileDataWriter::PrintScopedProfileToFile(ScopedProfileOutputPath, data);
	}
#endif // ENABLE_SCOPED_PROFILER
}

void ApplicationData::threadSaveProfileData([[maybe_unused]] size_t threadIndex)
{
#ifdef ENABLE_SCOPED_PROFILER
	std::lock_guard l(mScopedProfileRecordsMutex);
	mScopedProfileRecords.emplace_back(threadIndex, gtlScopedProfilerData.getAllRecords());
#endif // ENABLE_SCOPED_PROFILER
}

void ApplicationData::shutdownThreads()
{
	threadPool.shutdown();
	renderThread.shutdownThread();
	resourceManager.stopLoadingThread();
}
