#pragma once

#ifdef IMGUI_ENABLED

#include <mutex>
#include <memory>

#include <raccoon-ecs/utils/system.h>

#include "HAL/EngineFwd.h"

#include "GameLogic/Imgui/ImguiMainMenu.h"

class ImguiComponent;
struct ImguiDebugData;

/**
 * System that handles dear imgui debug tool
 */
class ImguiSystem final : public RaccoonEcs::System
{
public:
	explicit ImguiSystem(
		ImguiDebugData& debugData,
		HAL::Engine& engine
	) noexcept;

	void update() override;
	void init() override;
	void shutdown() override;

private:
	HAL::Engine& mEngine;
	ImguiDebugData& mDebugData;

	ImguiMainMenu mImguiMainMenu;

	std::mutex mRenderDataMutex;
	std::shared_ptr<bool> mHasPreviousFrameProcessedOnRenderThread;
};

#endif // IMGUI_ENABLED
