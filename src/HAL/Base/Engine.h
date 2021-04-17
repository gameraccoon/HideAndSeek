#pragma once

#include <memory>

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
		Engine(int windowWidth, int windowHeight) noexcept;

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		~Engine();

		Vector2D getMousePos() const;

		void start(IGame* game);
		void quit();

		Graphics::Renderer& getRenderer();

		Vector2D getWindowSize() const;

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
