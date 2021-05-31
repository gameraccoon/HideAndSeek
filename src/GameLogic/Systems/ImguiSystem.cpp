#include "Base/precomp.h"

#include "GameLogic/Systems/ImguiSystem.h"

#ifdef IMGUI_ENABLED

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "imgui/examples/imgui_impl_opengl2.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include "GameData/GameData.h"
#include "GameData/Components/ImguiComponent.generated.h"

#include "HAL/Base/Engine.h"

ImguiSystem::ImguiSystem(
		RaccoonEcs::ComponentAdder<ImguiComponent>&& imguiAdder,
		ImguiDebugData& debugData,
		HAL::Engine& engine) noexcept
	: mImguiAdder(std::move(imguiAdder))
	, mEngine(engine)
	, mDebugData(debugData)
{
}

void ImguiSystem::update()
{
	GameData& gameData = mDebugData.worldHolder.getGameData();

	// check if we need to render imgui
	ImguiComponent* imgui = mImguiAdder.getOrAddComponent(gameData.getGameComponents());

	if (!imgui->getIsImguiVisible())
	{
		return;
	}

	ImGui_ImplSDL2_ProcessEvent(&mEngine.getLastEventRef());

	// start the imgui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(mEngine.getRawWindow());
	ImGui::NewFrame();

	// update the window hierarchy
	mImguiMainMenu.update(mDebugData);

	// rendering imgui to the viewport
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void ImguiSystem::initResources()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(mEngine.getRawWindow(), mEngine.getRawGlContext());
	ImGui_ImplOpenGL2_Init();
}

void ImguiSystem::shutdown()
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

#endif // IMGUI_ENABLED
