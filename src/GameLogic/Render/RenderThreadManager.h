#pragma once

#include <memory>
#include <thread>

#include "GameLogic/Render/RenderAccessor.h"

class ResourceManager;

namespace HAL
{
	class Engine;
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

	void startThread(ResourceManager& resourceManager, HAL::Engine& engine, std::function<void()>&& threadInitializeFn);
	void shutdownThread();

	// temp code
	void testRunMainThread(RenderAccessor& renderAccessor, ResourceManager& resourceManager, HAL::Engine& engine);

private:
	using RenderDataVector = std::vector<std::unique_ptr<RenderData>>;

private:
	static void RenderThreadFunction(RenderAccessor& renderAccessor, ResourceManager& resourceManager, HAL::Engine& engine);
	static void ConsumeAndRenderQueue(RenderDataVector&& dataToRender, ResourceManager& resourceManager, HAL::Engine& engine);

private:
	// contains everything needed to communicate with the render thread
	RenderAccessor mRenderAccessor;
	std::unique_ptr<std::thread> mRenderThread;
};
