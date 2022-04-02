#include "Base/precomp.h"

#include "HAL/Base/Engine.h"

#include "Base/Debug/ConcurrentAccessDetector.h"

#include <algorithm>

#include <glew/glew.h>

#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "HAL/IGame.h"
#include "HAL/Internal/GlContext.h"
#include "HAL/Internal/Sdl.h"
#include "HAL/Internal/SdlWindow.h"

namespace HAL
{
#ifdef CONCURRENT_ACCESS_DETECTION
	ConcurrentAccessDetector gSDLAccessDetector;
#endif

	static const float ONE_SECOND_TICKS = 1000.0f;
	static const float ONE_TICK_SECONDS = 1.0f / ONE_SECOND_TICKS;
	static const Uint32 ONE_FRAME_TICKS = 16;
	static const float ONE_FRAME_SEC = ONE_FRAME_TICKS * ONE_TICK_SECONDS;

	struct Engine::Impl
	{
		Internal::SDLInstance mSdl;
		Internal::Window mWindow;
		Internal::GlContext mGlContext{mWindow};
		Graphics::Renderer mRenderer;
		Uint32 mLastFrameTicks;
		IGame* mGame = nullptr;

		float mMouseX;
		float mMouseY;
		SDL_Event mLastEvent;

		Impl(int windowWidth, int windowHeight) noexcept
			: mSdl(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)
			, mWindow(windowWidth, windowHeight)
			, mLastFrameTicks(SDL_GetTicks())
			, mMouseX(static_cast<float>(windowWidth) * 0.5f)
			, mMouseY(static_cast<float>(windowHeight) * 0.5f)
		{
		}

		~Impl() = default;

		void start();
		void parseEvents();
	};

	Engine::Engine(int windowWidth, int windowHeight) noexcept
		: WindowWidth(windowWidth)
		, WindowHeight(windowHeight)
		, mPimpl(HS_NEW Impl(windowWidth, windowHeight))
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetSwapInterval(0); // vsync off

		glEnable(GL_TEXTURE_2D);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, WindowWidth, WindowHeight, 0.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);

		if (TTF_Init() == -1)
		{
			ReportError("TTF_Init failed");
		}

		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			ReportError("TTF_Init failed");
		}
	}

	Engine::~Engine()
	{
		Mix_CloseAudio();
		TTF_Quit();
	}

	Vector2D Engine::getMousePos() const
	{
		return Vector2D(mPimpl->mMouseX, mPimpl->mMouseY);
	}

	void Engine::start(IGame* game)
	{
		mPimpl->mGame = game;
		mPimpl->mWindow.show();
		game->initResources();
		mPimpl->start();
	}

	Graphics::Renderer& Engine::getRenderer()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		return mPimpl->mRenderer;
	}

	Vector2D Engine::getWindowSize() const
	{
		return Vector2D(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight));
	}

	void Engine::releaseRenderContext()
	{
		SDL_GL_MakeCurrent(nullptr, 0);
	}

	void Engine::acquireRenderContext()
	{
		SDL_GL_MakeCurrent(mPimpl->mWindow.getRawWindow(), mPimpl->mGlContext.getRawGLContext());
	}

	SDL_Window* Engine::getRawWindow()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		return mPimpl->mWindow.getRawWindow();
	}

	void* Engine::getRawGlContext()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		return mPimpl->mGlContext.getRawGLContext();
	}

	SDL_Event& Engine::getLastEventRef()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		return mPimpl->mLastEvent;
	}

	void Engine::Impl::start()
	{
		while (!mGame->shouldQuitGame())
		{
			parseEvents();

			Uint32 currentTicks = static_cast<float>(SDL_GetTicks());

			// time wrapped around, start counting time to frame from now
			if (currentTicks < mLastFrameTicks) {
				mLastFrameTicks = currentTicks;
				continue;
			}

			Uint32 ticks = currentTicks - mLastFrameTicks;
			if (ticks >= ONE_FRAME_TICKS)
			{
				const float lastFrameDurationSec = ticks * ONE_TICK_SECONDS;

				if (mGame)
				{
					mGame->dynamicTimePreFrameUpdate(lastFrameDurationSec);
				}

				while (ticks >= ONE_FRAME_TICKS)
				{
					if (mGame)
					{
						mGame->fixedTimeUpdate(ONE_FRAME_SEC);
					}
					ticks -= ONE_FRAME_TICKS;
				}

				if (mGame)
				{
					mGame->dynamicTimePostFrameUpdate(lastFrameDurationSec);
				}
				mLastFrameTicks = currentTicks - ticks;
			}
		}
	}

	void Engine::Impl::parseEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
			case SDL_QUIT:
				mGame->quitGame();
				break;
			case SDL_KEYDOWN:
				mGame->setKeyboardKeyState(event.key.keysym.sym, true);
				break;
			case SDL_KEYUP:
				mGame->setKeyboardKeyState(event.key.keysym.sym, false);
				break;
			case SDL_MOUSEBUTTONDOWN:
				mGame->setMouseKeyState(event.button.button, true);
				break;
			case SDL_MOUSEBUTTONUP:
				mGame->setMouseKeyState(event.button.button, false);
				break;
			case SDL_MOUSEMOTION:
				mMouseX = static_cast<float>(event.motion.x);
				mMouseY = static_cast<float>(event.motion.y);
				break;
			default:
				break;
			}
		}
		mLastEvent = event;
	}
}
