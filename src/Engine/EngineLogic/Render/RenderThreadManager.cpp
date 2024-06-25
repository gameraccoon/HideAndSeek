#include "EngineCommon/precomp.h"

#include "EngineLogic/Render/RenderThreadManager.h"

#include <SDL_video.h>
#include <algorithm>
#include <bitset>
#include <glew/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>

#include "EngineCommon/Types/ComplexTypes/VectorUtils.h"

#include "HAL/Base/Math.h"
#include "HAL/Graphics/Renderer.h"
#include "HAL/Graphics/Sprite.h"
#include "HAL/Base/Engine.h"

#include "GameUtils/ResourceManagement/ResourceManager.h"

namespace RenderThreadManagerInternal
{
	static int gTotalGameInstancesCount = 1;
	static unsigned int gGameInstancesLeftBitset = (1u << gTotalGameInstancesCount) - 1u;

	class RenderVisitor
	{
	public:
		RenderVisitor(ResourceManager& resourceManager, HAL::Engine& engine, const Graphics::Surface*& lastSurface, const int gameInstanceIdx)
			: mResourceManager(resourceManager)
			, mEngine(engine)
			, mLastSurface(lastSurface)
			, mGameInstanceIdx(gameInstanceIdx)
		{}

		void operator()(BackgroundRenderData&& bgData)
		{
			SCOPED_PROFILER("RenderVisitor->BackgroundRenderData");

			const Graphics::Sprite* bgSprite = mResourceManager.tryGetResource<Graphics::Sprite>(bgData.spriteHandle);

			if (bgSprite == nullptr)
			{
				return;
			}

			bindSurface(bgSprite->getSurface());
			Graphics::Render::DrawTiledQuad(
				bgData.start,
				bgData.size,
				bgData.uv
			);
		}

		void operator()(FanRenderData&& fanData)
		{
			SCOPED_PROFILER("RenderVisitor->FanRenderData");

			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(fanData.spriteHandle);

			if (sprite == nullptr)
			{
				return;
			}

			const Graphics::QuadUV spriteUV = sprite->getUV();

			std::vector<Graphics::DrawPoint> points;
			points.reserve(fanData.points.size());
			const Vector2D backScale{1.0f/fanData.size.x, 1.0f/fanData.size.y};
			for (Vector2D point : fanData.points)
			{
				points.emplace_back(point, Graphics::QuadLerp(spriteUV, 0.5f+point.x*backScale.x, 0.5f+point.y*backScale.y));
			}

			const Vector2D drawShift = fanData.start + fanData.size;
			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
			bindSurface(sprite->getSurface());
			Graphics::Render::DrawFan(
				points,
				transform,
				fanData.alpha
			);
		}

		void operator()(QuadRenderData&& quadData)
		{
			SCOPED_PROFILER("RenderVisitor->QuadRenderData");

			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(quadData.spriteHandle);
			if (sprite == nullptr)
			{
				return;
			}

			bindSurface(sprite->getSurface());
			Graphics::Render::DrawQuad(
				quadData.position,
				quadData.size,
				quadData.anchor,
				quadData.rotation,
				sprite->getUV(),
				quadData.alpha
			);
		}

		void operator()(PolygonRenderData&& polygonData)
		{
			SCOPED_PROFILER("RenderVisitor->PolygonRenderData");

			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(polygonData.spriteHandle);

			if (sprite == nullptr)
			{
				return;
			}

			const Graphics::QuadUV spriteUV = sprite->getUV();

			for (Graphics::DrawPoint& point : polygonData.points)
			{
				point.texturePoint = QuadLerp(spriteUV, point.texturePoint);
			}

			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(polygonData.drawShift.x, polygonData.drawShift.y, 0.0f));
			bindSurface(sprite->getSurface());
			Graphics::Render::DrawFan(polygonData.points, transform, 0.3f);
		}

		void operator()(StripRenderData&& stripData)
		{
			SCOPED_PROFILER("RenderVisitor->StripRenderData");

			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(stripData.spriteHandle);
			if (sprite == nullptr)
			{
				return;
			}

			const Graphics::QuadUV spriteUV = sprite->getUV();

			for (Graphics::DrawPoint& point : stripData.points)
			{
				point.texturePoint = QuadLerp(spriteUV, point.texturePoint);
			}

			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(stripData.drawShift.x, stripData.drawShift.y, 0.0f));
			bindSurface(sprite->getSurface());
			Graphics::Render::DrawStrip(stripData.points, transform, stripData.alpha);
		}

		void operator()(const TextRenderData& /*textData*/)
		{
			SCOPED_PROFILER("RenderVisitor->TextRenderData");

			// need an implimentation when text rendering is fixed
		}

		void operator()(const CustomRenderFunction& renderFunction) const
		{
			SCOPED_PROFILER("RenderVisitor->CustomRenderFunction");

			renderFunction.renderThreadFn();
		}

		void operator()(const FinalizeFrameCommand&) const
		{
			SCOPED_PROFILER("RenderVisitor->SwapBuffersCommand");

			// wait for all the instances before swapping buffers
			gGameInstancesLeftBitset &= ~(1u << mGameInstanceIdx);
			if (gGameInstancesLeftBitset != 0)
			{
				return;
			}
			gGameInstancesLeftBitset = (1u << gTotalGameInstancesCount) - 1u;

			SDL_GL_SwapWindow(mEngine.getRawWindow());
			glClear(GL_COLOR_BUFFER_BIT);
		}

	private:
		void bindSurface(const Graphics::Surface* surface)
		{
			if (surface != mLastSurface)
			{
				mLastSurface = surface;
				Graphics::Render::BindSurface(*surface);
			}
		}

	private:
		ResourceManager& mResourceManager;
		HAL::Engine& mEngine;
		const Graphics::Surface*& mLastSurface;
		const int mGameInstanceIdx;
	};

	using DataPtr = std::unique_ptr<RenderData>;
	using FrameData = std::vector<DataPtr>;
	using InstanceFrames = std::vector<FrameData>;
	using DataToRender = std::vector<InstanceFrames>;

	static void PopulateFrameData(DataToRender& outData, std::vector<DataPtr>& inDataToTrasfer)
	{
		SCOPED_PROFILER("PopulateFrameData");

		outData.resize(gTotalGameInstancesCount);
		for (DataPtr& operationsBulk : inDataToTrasfer)
		{
			if (!operationsBulk->layers.empty())
			{
				InstanceFrames& instanceData = outData[operationsBulk->gameInstanceIndex];

				if (instanceData.empty())
				{
					instanceData.emplace_back();
				}

				const bool containsFinalization = std::ranges::any_of(operationsBulk->layers, [](const RenderData::Layer& layer)
				{
					return std::holds_alternative<FinalizeFrameCommand>(layer);
				});

				instanceData.back().push_back(std::move(operationsBulk));

				if (containsFinalization)
				{
					// always have a frame after finalization
					// even if it's empty
					instanceData.emplace_back();
				}
			}
		}
	}

	static void ProcessRenderQueue(DataToRender& dataToRender, ResourceManager& resourceManager, HAL::Engine& engine)
	{
		SCOPED_PROFILER("ProcessRenderQueue");

		for (const InstanceFrames& frames : dataToRender)
		{
			if (frames.size() < 2)
			{
				// too early to continue if at least one game instance doesn't have two frames
				// (we keep one frame after finalization, even if it's empty)
				return;
			}
		}

		const Graphics::Surface* lastSurface = nullptr;

		// only render the frame before the last
		for (const InstanceFrames& frames : dataToRender)
		{
			const FrameData& lastCompleteFrame = frames[frames.size() - 2];
			for (const DataPtr& renderData : lastCompleteFrame)
			{
				for (RenderData::Layer& layer : renderData->layers)
				{
					std::visit(RenderVisitor{resourceManager, engine, lastSurface, renderData->gameInstanceIndex}, std::move(layer));
				}
			}
		}

#ifdef DEBUG_CHECKS
		for (size_t i = 0; i < dataToRender.size(); ++i)
		{
			const InstanceFrames& frames = dataToRender[i];
			if (frames.size() > 2)
			{
				LogWarning("Render frame skipped. Count: %d Game instance: %u", frames.size() - 2, i);
			}
		}
#endif

		// remove all frames before the last
		for (InstanceFrames& frames : dataToRender)
		{
			frames.erase(frames.begin(), frames.begin() + (frames.size() - 1));
		}
	}
}

RenderThreadManager::~RenderThreadManager()
{
	shutdownThread();
}

void RenderThreadManager::startThread(ResourceManager& resourceManager, HAL::Engine& engine, std::function<void()>&& threadInitializeFn)
{
	mRenderThread = std::make_unique<std::thread>(
		[&renderAccessor = mRenderAccessor, &resourceManager, &engine, threadInitializeFn]
		{
			if (threadInitializeFn)
			{
				threadInitializeFn();
			}

			// prepare the buffer to render the first frame
			glClear(GL_COLOR_BUFFER_BIT);

			RenderThreadFunction(renderAccessor, resourceManager, engine);
		}
	);
}

void RenderThreadManager::shutdownThread()
{
	{
		std::lock_guard l(mRenderAccessor.dataMutex);
		mRenderAccessor.shutdownRequested = true;
	}
	mRenderAccessor.notifyRenderThread.notify_all();
	if (mRenderThread)
	{
		mRenderThread->join();
		mRenderThread = nullptr;
	}
}

void RenderThreadManager::testRunMainThread(RenderAccessor& renderAccessor, ResourceManager& resourceManager, HAL::Engine& engine)
{
	using namespace RenderThreadManagerInternal;

	DataToRender dataToRender;
	PopulateFrameData(dataToRender, renderAccessor.dataToTransfer);
	resourceManager.runThreadTasks(Resource::Thread::Render);
	ProcessRenderQueue(dataToRender, resourceManager, engine);
}

void RenderThreadManager::setAmountOfRenderedGameInstances(int instancesCount)
{
	using namespace RenderThreadManagerInternal;

	if (gTotalGameInstancesCount != instancesCount)
	{
		if (gGameInstancesLeftBitset == (1u << gTotalGameInstancesCount) - 1)
		{
			gGameInstancesLeftBitset = (1u << instancesCount) - 1;
		}
	}
	gTotalGameInstancesCount = instancesCount;
}

void RenderThreadManager::RenderThreadFunction(RenderAccessor& renderAccessor, ResourceManager& resourceManager, HAL::Engine& engine)
{
	using namespace RenderThreadManagerInternal;

	DataToRender dataToRender;
	while(true)
	{
		{
			std::unique_lock lock(renderAccessor.dataMutex);
			renderAccessor.notifyRenderThread.wait(lock, [&renderAccessor]{
				return renderAccessor.shutdownRequested || !renderAccessor.dataToTransfer.empty();
			});

			if (renderAccessor.shutdownRequested)
			{
				break;
			}

			PopulateFrameData(dataToRender, renderAccessor.dataToTransfer);
			renderAccessor.dataToTransfer.clear();
		}

		resourceManager.runThreadTasks(Resource::Thread::Render);
		ProcessRenderQueue(dataToRender, resourceManager, engine);
	}

#ifdef ENABLE_SCOPED_PROFILER
	renderAccessor.scopedProfilerRecords = gtlScopedProfilerData.getAllRecords();
#endif // ENABLE_SCOPED_PROFILER
}
