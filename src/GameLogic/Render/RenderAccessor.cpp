#include "Base/precomp.h"

#include "GameLogic/Render/RenderAccessor.h"


void RenderAccessor::submitData(std::unique_ptr<RenderData>&& newData)
{
	{
		std::lock_guard lock(dataMutex);
		dataToTransfer.push_back(std::move(newData));
	}

	notifyRenderThread.notify_all();
}