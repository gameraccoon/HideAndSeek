#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>
#include <functional>

#include "Base/Profile/ScopedProfiler.h"

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Render/RenderAccessorGameRef.h"

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

struct StripRenderData
{
	std::vector<Graphics::DrawPoint> points;
	ResourceHandle spriteHandle;
	Vector2D drawShift;
	float alpha = 1.0f;
};

struct PolygonRenderData
{
	std::vector<Graphics::DrawPoint> points;
	ResourceHandle spriteHandle;
	Vector2D drawShift;
	float alpha = 1.0f;
};

struct TextRenderData
{
	std::string text;
	Vector2D pos;
	ResourceHandle fontHandle;
	Graphics::Color color;
};

struct CustomRenderFunction
{
	std::function<void()> renderThreadFn;
};

struct FinalizeFrameCommand
{
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
		CustomRenderFunction,
		FinalizeFrameCommand
	>;

	RenderData() = default;

	RenderData(std::vector<Layer>&& layers)
		: layers(layers)
	{}

	std::vector<Layer> layers;
	int gameInstanceIndex = 0;
};

class RenderAccessor : public IRenderAccessor
{
	friend class RenderThreadManager;

public:
	RenderAccessor() = default;
	~RenderAccessor() = default;
	RenderAccessor(RenderAccessor&) = delete;
	RenderAccessor& operator=(RenderAccessor&) = delete;
	RenderAccessor(RenderAccessor&&) = delete;
	RenderAccessor& operator=(RenderAccessor&&) = delete;

	void submitData(std::unique_ptr<RenderData>&& newData, int gameInstanceIndex) override;

#ifdef ENABLE_SCOPED_PROFILER
	ScopedProfilerThreadData::Records consumeScopedProfilerRecordsUnsafe();
#endif

private:
	std::mutex dataMutex;
	bool shutdownRequested = false;
	std::vector<std::unique_ptr<RenderData>> dataToTransfer;
	std::condition_variable notifyRenderThread;

#ifdef ENABLE_SCOPED_PROFILER
	ScopedProfilerThreadData::Records scopedProfilerRecords;
#endif
};
