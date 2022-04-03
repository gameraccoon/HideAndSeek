#pragma once

#include <memory>
#include <chrono>

#include "Base/Types/BasicTypes.h"

#include "HAL/Graphics/Renderer.h"
#include "HAL/Base/Types.h"

struct SDL_Window;
union SDL_Event;

namespace HAL
{
	class IGame;

	class Engine
	{
	public:
		// note that instead of counting in fractions of seconds we count in integral milliseconds/ticks
		// so the frame rate is not exactly 60 FPS and the frame time is not exactly 1/60 sec
		static constexpr int ONE_SECOND_TICKS = 1000;
		static constexpr float ONE_TICK_SECONDS = 1.0f / ONE_SECOND_TICKS;
		static constexpr u32 ONE_FRAME_TICKS = 16;
		static constexpr float ONE_FRAME_SEC = ONE_FRAME_TICKS * ONE_TICK_SECONDS;

		static constexpr std::chrono::duration ONE_FRAME_DURATION = std::chrono::milliseconds{16};

	public:
		Engine(int windowWidth, int windowHeight) noexcept;

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		~Engine();

		Vector2D getMousePos() const;

		void start(IGame* game);

		Graphics::Renderer& getRenderer();

		Vector2D getWindowSize() const;

		void releaseRenderContext();
		void acquireRenderContext();

		// for debug tools such as imgui
		SDL_Window* getRawWindow();
		void* getRawGlContext();
		SDL_Event& getLastEventRef();

	private:
		const int WindowWidth;
		const int WindowHeight;

		struct Impl;
		std::unique_ptr<Impl> mPimpl;
	};
}
