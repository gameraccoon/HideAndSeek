#pragma once

#ifdef IMGUI_ENABLED

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

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
		RaccoonEcs::ComponentAdder<ImguiComponent>&& imguiAdder,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent> renderAccessorFilter,
		RaccoonEcs::InnerDataAccessor&& innerDataAccessor,
		ImguiDebugData& debugData,
		HAL::Engine& engine) noexcept;
	~ImguiSystem() override = default;

	void update() override;
	void initResources() override;
	void shutdown() override;
	static std::string GetSystemId() { return "ImguiSystem"; }

private:
	RaccoonEcs::ComponentAdder<ImguiComponent> mImguiAdder;
	RaccoonEcs::ComponentFilter<RenderAccessorComponent> mRenderAccessorFilter;
	RaccoonEcs::InnerDataAccessor mInnerDataAccessor;
	HAL::Engine& mEngine;
	ImguiDebugData& mDebugData;

	ImguiMainMenu mImguiMainMenu;
};

#endif // IMGUI_ENABLED
