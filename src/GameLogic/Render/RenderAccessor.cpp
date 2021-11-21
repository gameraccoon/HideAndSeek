#include "Base/precomp.h"

#include "GameLogic/Render/RenderAccessor.h"


void RenderAccessor::submitData(std::unique_ptr<RenderData>&& newData)
{
	{
		std::lock_guard l(dataMutex);
		dataToTransfer.push_back(std::move(newData));
	}

	notifyRenderThread.notify_all();
}

#ifdef ENABLE_SCOPED_PROFILER
ScopedProfilerThreadData::Records RenderAccessor::consumeScopedProfilerRecordsUnsafe()
{
	return std::move(scopedProfilerRecords);
}
#endif // ENABLE_SCOPED_PROFILER
