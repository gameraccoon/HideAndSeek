#pragma once

#include <memory>
#include <vector>

#ifdef IMGUI_ENABLED
#include "ECS/System.h"

#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/Imgui/ImguiDebugData.h"

#include "GameLogic/Imgui/ImguiMainMenu.h"


/**
 * System that handles dear imgui debug tool
 */
class ImguiSystem : public Ecs::System
{
public:
	ImguiSystem(ImguiDebugData& debugData, HAL::Engine& engine) noexcept;
	~ImguiSystem() override = default;

	void update() override;
	void initResources() override;
	void shutdown() override;
	std::string getName() override { return "ImguiSystem"; }

private:
	HAL::Engine& mEngine;
	ImguiDebugData& mDebugData;

	ImguiMainMenu mImguiMainMenu;
};

#endif // IMGUI_ENABLED
