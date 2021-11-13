#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

#ifdef RACCOON_ECS_PROFILE_SYSTEMS
#include <array>
#include <chrono>
#endif // RACCOON_ECS_PROFILE_SYSTEMS

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
		SynchroneousRenderData,
		SwapBuffersCommand
	>;

	RenderData() = default;

	RenderData(std::vector<Layer>&& layers)
		: layers(layers)
	{}

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


#ifdef RACCOON_ECS_PROFILE_SYSTEMS
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
	using WorkTimeRecords = std::vector<std::pair<TimePoint, TimePoint>>;
	// can be safely called only when render thread is stopped
	WorkTimeRecords consumeRenderWorkTimeUnsafe();
	ScopedProfilerThreadData::Records consumeScopedProfilerRecordsUnsafe();
#endif

private:
	std::mutex dataMutex;
	bool shutdownRequested = false;
	std::vector<std::unique_ptr<RenderData>> dataToTransfer;
	std::condition_variable notifyRenderThread;

#ifdef RACCOON_ECS_PROFILE_SYSTEMS
	WorkTimeRecords renderWorkTime;
	ScopedProfilerThreadData::Records scopedProfilerRecords;
#endif
};
