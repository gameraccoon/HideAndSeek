#pragma once

struct SDL_Window;

namespace HAL
{
	namespace Internal
	{
		class Window
		{
		public:
			Window(int width, int height);

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;
			Window(Window&&) = delete;
			Window& operator=(Window&&) = delete;
			~Window();

			SDL_Window* getRawWindow();
			void show();

		private:
			SDL_Window* mSDLWindow;
		};
	}
}
