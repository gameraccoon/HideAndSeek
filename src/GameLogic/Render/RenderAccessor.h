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

struct FanRenderData
{
	std::vector<Vector2D> points;
	ResourceHandle spriteHandle;
	Vector2D start;
	Vector2D size;
	float alpha = 1.0f;
};

struct QuadRenderData
{
	ResourceHandle spriteHandle;
	Vector2D position;
	Vector2D size;
	Vector2D anchor = {0.5f, 0.5f};
	float rotation = 0.0f;
	float alpha = 1.0f;
};

struct StripRenderData {
	std::vector<Graphics::DrawPoint> points;
	ResourceHandle spriteHandle;
	Vector2D drawShift;
	float alpha = 1.0f;
};

struct PolygonRenderData {
	std::vector<Graphics::DrawPoint> points;
	ResourceHandle spriteHandle;
	Vector2D drawShift;
	float alpha = 1.0f;
};

struct TextRenderData {
	std::string text;
	Vector2D pos;
	ResourceHandle fontHandle;
	Graphics::Color color;
};

struct SyncRenderSharedData {
	std::condition_variable onFinished;
	std::mutex isFinishedMutex;
	bool isFinised = false;
};
struct SynchroneousRenderData {
	std::shared_ptr<SyncRenderSharedData> sharedData;
	std::function<void()> renderThreadFn;
};

struct SwapBuffersCommand {
};

struct RenderData
{
	using Layer = std::variant<
		BackgroundRenderData,
		FanRenderData,
		QuadRenderData,
		StripRenderData,
		PolygonRenderData,
		TextRenderData,
		SynchroneousRenderData
	>;

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
