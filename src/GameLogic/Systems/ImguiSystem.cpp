#include "Base/precomp.h"

#include "GameLogic/Systems/ImguiSystem.h"

#ifdef IMGUI_ENABLED

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "imgui/examples/imgui_impl_opengl2.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include "Base/Types/TemplateHelpers.h"

#include "GameData/GameData.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Render/RenderAccessor.h"


ImguiSystem::ImguiSystem(
		ImguiDebugData& debugData,
		HAL::Engine& engine) noexcept
	: mEngine(engine)
	, mDebugData(debugData)
{
}

void ImguiSystem::update()
{
	SCOPED_PROFILER("ImguiSystem::update");
	GameData& gameData = mDebugData.worldHolder.getGameData();

	// check if we need to render imgui
	ImguiComponent* imgui = gameData.getGameComponents().getOrAddComponent<ImguiComponent>();

	if (!imgui->getIsImguiVisible())
	{
		return;
	}

	RenderAccessor* renderAccessor = nullptr;
	if (auto [renderAccessorCmp] = gameData.getGameComponents().getComponents<RenderAccessorComponent>(); renderAccessorCmp != nullptr)
	{
		renderAccessor = renderAccessorCmp->getAccessor();
	}

	if (renderAccessor == nullptr)
	{
		return;
	}

	std::unique_ptr<RenderData> renderData = std::make_unique<RenderData>();
	SynchroneousRenderData& syncData = TemplateHelpers::EmplaceVariant<SynchroneousRenderData>(renderData->layers);

	syncData.renderThreadFn = [this] {
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
	};
	syncData.sharedData = std::make_shared<SyncRenderSharedData>();

	// have a owning copy of the data before moving render data to the render thread
	std::shared_ptr<SyncRenderSharedData> sharedData = syncData.sharedData;

	renderAccessor->submitData(std::move(renderData));

	// wait until we finish rendering
	std::unique_lock lock(sharedData->isFinishedMutex);
	sharedData->onFinished.wait(lock, [&sharedData]{ return sharedData->isFinised; });
}

void ImguiSystem::init()
{
	SCOPED_PROFILER("ImguiSystem::initResources");

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
	SCOPED_PROFILER("ImguiSystem::shutdown");
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

#endif // IMGUI_ENABLED
