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

private:
	static void RenderThreadFunction(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager);

private:
	// contains everything needed to communicate with the render thread
	RenderAccessor mRenderAccessor;
	std::unique_ptr<std::thread> mRenderThread;
};
