#pragma once

#include <memory>
#include <thread>

#include "GameLogic/Render/RenderAccessor.h"

namespace HAL
{
	class ResourceManager;
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

	void startThread(HAL::ResourceManager& resourceManager, HAL::Engine& engine, std::function<void()>&& threadInitializeFn);

	// temp code
	void testRunMainThread(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager, HAL::Engine& engine);

private:
	using RenderDataVector = std::vector<std::unique_ptr<RenderData>>;

private:
	static void RenderThreadFunction(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager, HAL::Engine& engine);
	static void ConsumeAndRenderQueue(RenderDataVector&& dataToRender, HAL::ResourceManager& resourceManager, HAL::Engine& engine);

private:
	// contains everything needed to communicate with the render thread
	RenderAccessor mRenderAccessor;
	std::unique_ptr<std::thread> mRenderThread;
};
