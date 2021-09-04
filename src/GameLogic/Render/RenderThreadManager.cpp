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

void RenderThreadManager::startThread(HAL::ResourceManager& /*resourceManager*/, std::function<void()>&& /*threadInitializeFn*/)
{
/*	mRenderThread = std::make_unique<std::thread>(
		[&renderAccessor = mRenderAccessor, &resourceManager, threadInitializeFn]
		{
			if (threadInitializeFn)
			{
				threadInitializeFn();
			}
			RenderThreadManager::RenderThreadFunction(renderAccessor, resourceManager);
		}
	);*/
}

namespace RenderThreadManagerInternal
{
	class RenderVisitor
	{
	public:
		RenderVisitor(HAL::ResourceManager& resourceManager)
			: mResourceManager(resourceManager)
		{}

		void operator()(BackgroundRenderData&& bgData)
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

		void operator()(FanRenderData&& fanData)
		{
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
			Graphics::Render::DrawFan(
				*sprite->getSurface(),
				points,
				transform,
				fanData.alpha
			);
		}

		void operator()(QuadRenderData&& quadData)
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

		void operator()(PolygonRenderData&& polygonData)
		{
			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(polygonData.spriteHandle);

			if (sprite == nullptr)
			{
				return;
			}

			Graphics::QuadUV spriteUV = sprite->getUV();

			for (Graphics::DrawPoint& point : polygonData.points)
			{
				point.texturePoint = Graphics::QuadLerp(spriteUV, point.texturePoint);
			}

			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(polygonData.drawShift.x, polygonData.drawShift.y, 0.0f));
			Graphics::Render::DrawFan(*sprite->getSurface(), polygonData.points, transform, 0.3f);
		}

		void operator()(StripRenderData&& stripData)
		{
			const Graphics::Sprite* sprite = mResourceManager.tryGetResource<Graphics::Sprite>(stripData.spriteHandle);
			if (sprite == nullptr)
			{
				return;
			}

			Graphics::QuadUV spriteUV = sprite->getUV();

			for (Graphics::DrawPoint& point : stripData.points)
			{
				point.texturePoint = Graphics::QuadLerp(spriteUV, point.texturePoint);
			}

			glm::mat4 transform(1.0f);
			transform = glm::translate(transform, glm::vec3(stripData.drawShift.x, stripData.drawShift.y, 0.0f));
			Graphics::Render::DrawStrip(*sprite->getSurface(), stripData.points, transform, stripData.alpha);
		}

		void operator()(const TextRenderData& /*textData*/)
		{
			// need an implimentation when text rendering is fixed
		}

	private:
		HAL::ResourceManager& mResourceManager;
	};
}

void RenderThreadManager::RenderThreadFunction(RenderAccessor& renderAccessor, HAL::ResourceManager& resourceManager)
{
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

			TransferDataToQueue(dataToRender, renderAccessor.dataToTransfer);
		}

		ConsumeAndRenderQueue(std::move(dataToRender), resourceManager);
	}
}

void RenderThreadManager::TransferDataToQueue(RenderDataVector& inOutDataToRender, RenderDataVector& inOutDataToTransfer)
{
	if (inOutDataToRender.empty())
	{
		inOutDataToRender = std::move(inOutDataToTransfer);
	}
	else
	{
		inOutDataToRender.reserve(inOutDataToRender.size() + inOutDataToTransfer.size());
		std::move(
			inOutDataToTransfer.begin(),
			inOutDataToTransfer.end(),
			std::back_inserter(inOutDataToRender)
		);
		inOutDataToTransfer.clear();
	}
}

void RenderThreadManager::ConsumeAndRenderQueue(RenderDataVector&& dataToRender, HAL::ResourceManager& resourceManager)
{
	using namespace RenderThreadManagerInternal;
	for (std::unique_ptr<RenderData>& renderData : dataToRender)
	{
		for (RenderData::Layer& layer : renderData->layers)
		{
			std::visit(RenderVisitor{resourceManager}, std::move(layer));
		}
	}
	dataToRender.clear();
}
