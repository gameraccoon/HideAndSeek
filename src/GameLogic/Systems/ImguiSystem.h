#pragma once

#ifdef IMGUI_ENABLED

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>

#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/Imgui/ImguiDebugData.h"

#include "GameLogic/Imgui/ImguiMainMenu.h"

class ImguiComponent;

/**
 * System that handles dear imgui debug tool
 */
class ImguiSystem : public RaccoonEcs::System
{
public:
	ImguiSystem(
		ImguiDebugData& debugData,
		HAL::Engine& engine) noexcept;
	~ImguiSystem() override = default;

	void update() override;
	void init() override;
	void shutdown() override;
	static std::string GetSystemId() { return "ImguiSystem"; }

private:
	HAL::Engine& mEngine;
	ImguiDebugData& mDebugData;

	ImguiMainMenu mImguiMainMenu;
};

#endif // IMGUI_ENABLED
