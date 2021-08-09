#include "Base/precomp.h"

#include "GameLogic/Render/RenderThreadManager.h"

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "HAL/Base/Math.h"
#include "HAL/Base/ResourceManager.h"
#include "HAL/Graphics/Renderer.h"
#include "HAL/Graphics/Sprite.h"

RenderThreadManager::~RenderThreadManager()
{
	{
		std::lock_guard l(mRenderAccessor.dataMutex);
		mRenderAccessor.shutdownRequested = true;
	}
	mRenderAccessor.notifyRenderThread.notify_all();
	mRenderThread->join();
}

void RenderThreadManager::startThread(HAL::ResourceManager& resourceManager, std::function<void()>&& threadInitializeFn)
{
	mRenderThread = std::make_unique<std::thread>(
		[&renderAccessor = mRenderAccessor, &resourceManager, threadInitializeFn]
		{
			if (threadInitializeFn)
			{
				threadInitializeFn();
			}
			RenderThreadManager::RenderThreadFunction(renderAccessor, resourceManager);
		}
	);
}

namespace RenderThreadManagerInternal
{
	class RenderVisitor
	{
	public:
		RenderVisitor(HAL::ResourceManager& resourceManager)
			: mResourceManager(resourceManager)
		{}

		void operator()(const BackgroundRenderData& bgData)
		{
			const Graphics::Sprite* bgSprite = mResourceManager.tryGetResource<Graphics::Sprite>(bgData.spriteHandle);

			if (bgSprite == nullptr)
			{
				return;
			}

			Graphics::Render::DrawTiledQuad(
				*bgSprite->getSurface(),
				bgData.start,
				bgData.size,
				bgData.uv
			);
		}

		void operator()(const LightPolygonRenderData& lightPolygonData)
		{
			const Graphics::Sprite* lightSprite = mResourceManager.tryGetResource<Graphics::Sprite>(lightPolygonData.lightSpriteHandle);

			if (lightSprite == nullptr)
			{
				return;
			}

			const Graphics::QuadUV quadUV = lightSprite->getUV();

			std::vector<Graphics::DrawPoint> points;
			points.reserve(lightPolygonData.points.size());
			for (Vector2D point : lightPolygonData.points)
			{
				points.emplace_back(point, Graphics::QuadLerp(quadUV, 0.5f+point.x/lightPolygonData.textureWorldSize.x, 0.5f+point.y/lightPolygonData.textureWorldSize.y));
			}

			const Vector2D drawShift = lightPolygonData.boundsStart + lightPolygonData.textureWorldSize;
			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
			Graphics::Render::DrawFan(
				*lightSprite->getSurface(),
				points,
				transform,
				lightPolygonData.alpha
			);
		}

		void operator()(const QuadRenderData& quadData)
		{
			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(quadData.spriteHandle);
			if (sprite == nullptr)
			{
				return;
			}

			Graphics::Render::DrawQuad(
				*sprite->getSurface(),
				quadData.position,
				quadData.size,
				quadData.anchor,
				quadData.rotation,
				sprite->getUV(),
				quadData.alpha
			);
		}

	private:
		HAL::ResourceManager& mResourceManager;
	};
}

void RenderThreadManager::RenderThreadFunction(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager)
{
	using namespace RenderThreadManagerInternal;

	std::vector<std::unique_ptr<RenderData>> dataToRender;
	while(true)
	{
		{
			std::unique_lock lock(renderAccessor.dataMutex);
			renderAccessor.notifyRenderThread.wait(lock, [&renderAccessor]{
				return renderAccessor.shutdownRequested || !renderAccessor.dataToTransfer.empty();
			});

			if (renderAccessor.shutdownRequested)
			{
				return;
			}

			dataToRender.reserve(dataToRender.size() + renderAccessor.dataToTransfer.size());
			std::move(
				renderAccessor.dataToTransfer.begin(),
				renderAccessor.dataToTransfer.end(),
				std::back_inserter(dataToRender)
			);
			renderAccessor.dataToTransfer.clear();
		}

		for (std::unique_ptr<RenderData>& renderData : dataToRender)
		{
			for (RenderData::Layer& layer : renderData->layers)
			{
				std::visit(RenderVisitor{resourceManager}, layer);
			}
		}
		dataToRender.clear();
	}
}
