#pragma once

#include <memory>
#include <thread>

#include "GameLogic/Render/RenderAccessor.h"

namespace HAL
{
	class ResourceManager;
}

class RenderThreadManager
{
public:
	RenderThreadManager() = default;
	~RenderThreadManager();
	RenderThreadManager(RenderThreadManager&) = delete;
	RenderThreadManager& operator=(RenderThreadManager&) = delete;
	RenderThreadManager(RenderThreadManager&&) = delete;
	RenderThreadManager& operator=(RenderThreadManager&&) = delete;

	RenderAccessor& getAccessor() { return mRenderAccessor; }

	void startThread(HAL::ResourceManager& resourceManager, std::function<void()>&& threadInitializeFn);

	// temp code
	void testRunMainThread(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager)
	{
		RenderDataVector dataToRender;
		TransferDataToQueue(dataToRender, renderAccessor.dataToTransfer);
		ConsumeAndRenderQueue(std::move(dataToRender), resourceManager);
	}

private:
	using RenderDataVector = std::vector<std::unique_ptr<RenderData>>;
private:
	static void RenderThreadFunction(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager);
	static void TransferDataToQueue(RenderDataVector& inOutDataToRender, RenderDataVector& inOutDataToTransfer);
	static void ConsumeAndRenderQueue(RenderDataVector&& dataToRender, HAL::ResourceManager& resourceManager);

private:
	// contains everything needed to communicate with the render thread
	RenderAccessor mRenderAccessor;
	std::unique_ptr<std::thread> mRenderThread;
};
