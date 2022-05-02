#include "Base/precomp.h"

#include "HAL/Base/Engine.h"

#include <algorithm>
#include <thread>

#include <glew/glew.h>

#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "HAL/IGame.h"
#include "HAL/InputControllersData.h"
#include "HAL/Internal/GlContext.h"
#include "HAL/Internal/Sdl.h"
#include "HAL/Internal/SdlWindow.h"

namespace HAL
{
#ifdef CONCURRENT_ACCESS_DETECTION
	ConcurrentAccessDetector gSDLAccessDetector;
#endif

	struct Engine::Impl
	{
		Internal::SDLInstance mSdl;
		const int mWindowWidth;
		const int mWindowHeight;
		Internal::Window mWindow;
		Internal::GlContext mGlContext{mWindow};
		Graphics::Renderer mRenderer;
		IGame* mGame = nullptr;
		InputControllersData* mInputDataPtr = nullptr;

		std::vector<SDL_Event> mLastFrameEvents;

		Impl(int windowWidth, int windowHeight) noexcept
			: mSdl(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)
			, mWindowWidth(windowWidth)
			, mWindowHeight(windowHeight)
			, mWindow(windowWidth, windowHeight)
		{
		}

		~Impl() = default;

		void start();
		void parseEvents();
	};

	Engine::Engine(int windowWidth, int windowHeight) noexcept
		: mPimpl(HS_NEW Impl(windowWidth, windowHeight))
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
		glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 1.0);
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

	void Engine::start(IGame* game, InputControllersData* inputData)
	{
		mPimpl->mGame = game;
		mPimpl->mInputDataPtr = inputData;
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
		return Vector2D(static_cast<float>(mPimpl->mWindowWidth), static_cast<float>(mPimpl->mWindowHeight));
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

	std::vector<SDL_Event>& Engine::getLastFrameEvents()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		return mPimpl->mLastFrameEvents;
	}

	void Engine::Impl::start()
	{
		AssertFatal(mGame, "Game should be set to Engine before calling start()");
		// we advance the time a bit differently, fixed frame time is advanced in steps
		Uint64 lastFixedFrameTicks = SDL_GetTicks64();
		// and real time is update with the exact time passed from last processed frame
		Uint64 lastRealFrameTicks = SDL_GetTicks64();
		while (!mGame->shouldQuitGame())
		{
			parseEvents();

			Uint64 currentTicks = SDL_GetTicks64();

			// time was adjusted to past or wrapped around type, start counting time to frame from now
			if (currentTicks < lastFixedFrameTicks || currentTicks < lastRealFrameTicks)
			{
				lastFixedFrameTicks = currentTicks;
				lastRealFrameTicks = currentTicks;
				continue;
			}

			int iterations = 0;
			Uint64 fixedFrameticksLeft = currentTicks - lastFixedFrameTicks;
			if (fixedFrameticksLeft >= ONE_FIXED_UPDATE_TICKS)
			{
				Uint64 passedRealTicks = currentTicks - lastRealFrameTicks;
				// if we exceeded max frame ticks last frame, that likely mean we were staying on a breakpoint
				// readjust to normal ticking speed
				if (fixedFrameticksLeft > MAX_FRAME_TICKS)
				{
					LogInfo("Continued from a breakpoint or had a huge lag");
					fixedFrameticksLeft = ONE_FIXED_UPDATE_TICKS;
					passedRealTicks = ONE_FIXED_UPDATE_TICKS;
				}

				const float lastFrameDurationSec = passedRealTicks * ONE_TICK_SECONDS;

				while (fixedFrameticksLeft >= ONE_FIXED_UPDATE_TICKS)
				{
					fixedFrameticksLeft -= ONE_FIXED_UPDATE_TICKS;
					++iterations;
				}

				mGame->dynamicTimePreFrameUpdate(lastFrameDurationSec, iterations);
				for (int i = 0; i < iterations; ++i)
				{
					mGame->fixedTimeUpdate(ONE_FIXED_UPDATE_SEC);
				}
				mGame->dynamicTimePostFrameUpdate(lastFrameDurationSec, iterations);

				lastFixedFrameTicks = currentTicks - fixedFrameticksLeft;
				lastRealFrameTicks = currentTicks;

				mLastFrameEvents.clear();
			}

			if (iterations <= 1)
			{
				// give some time to other threads while waiting
				std::this_thread::yield();
			}
		}
	}

	void Engine::Impl::parseEvents()
	{
		SCOPED_PROFILER("Engine::Impl::parseEvents");
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type > SDL_LASTEVENT)
			{
				continue;
			}

			switch (event.type)
			{
			case SDL_QUIT:
				mGame->quitGame();
				break;
			case SDL_KEYDOWN:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates[Input::ControllerType::Keyboard].updateButtonState(event.key.keysym.sym, true);
				}
				break;
			case SDL_KEYUP:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates[Input::ControllerType::Keyboard].updateButtonState(event.key.keysym.sym, false);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates[Input::ControllerType::Mouse].updateButtonState(event.button.button, true);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates[Input::ControllerType::Mouse].updateButtonState(event.button.button, false);
				}
				break;
			case SDL_MOUSEMOTION:
				if (mInputDataPtr)
				{
					const Vector2D windowSize{static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)};
					const Vector2D mouseRelativePos{
						(event.motion.x / windowSize.x) * 2.0f - 1.0f,
						(event.motion.y / windowSize.y) * 2.0f - 1.0f
					};
					mInputDataPtr->controllerStates[Input::ControllerType::Mouse].updateAxis(0, mouseRelativePos.x);
					mInputDataPtr->controllerStates[Input::ControllerType::Mouse].updateAxis(1, mouseRelativePos.y);
				}
				break;
			default:
				break;
			}

			mLastFrameEvents.push_back(event);
		}
	}
}
