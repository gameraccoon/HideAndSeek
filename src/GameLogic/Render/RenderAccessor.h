#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

#include "GameData/Core/Vector2D.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/Base/Types.h"

struct BackgroundRenderData
{
	ResourceHandle spriteHandle;
	Vector2D start;
	Vector2D size;
	Graphics::QuadUV uv;
};

struct LightPolygonRenderData
{
	ResourceHandle lightSpriteHandle;
	std::vector<Vector2D> points;
	Vector2D boundsStart;
	Vector2D textureWorldSize;
	float alpha;
};

struct QuadRenderData
{
	ResourceHandle spriteHandle;
	Vector2D position;
	Vector2D size;
	Vector2D anchor;
	float rotation;
	float alpha;
};

struct RenderData
{
	using Layer = std::variant<BackgroundRenderData, LightPolygonRenderData, QuadRenderData>;

	std::vector<Layer> layers;
};

class RenderAccessor
{
	friend class RenderThreadManager;

public:
	RenderAccessor() = default;
	~RenderAccessor() = default;
	RenderAccessor(RenderAccessor&) = delete;
	RenderAccessor& operator=(RenderAccessor&) = delete;
	RenderAccessor(RenderAccessor&&) = delete;
	RenderAccessor& operator=(RenderAccessor&&) = delete;

	void submitData(std::unique_ptr<RenderData>&& newData);

private:
	std::mutex dataMutex;
	bool shutdownRequested = false;
	std::vector<std::unique_ptr<RenderData>> dataToTransfer;
	std::condition_variable notifyRenderThread;
};
