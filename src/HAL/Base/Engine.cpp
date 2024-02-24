#include "Base/precomp.h"

#include "HAL/Base/Engine.h"

#include <algorithm>

#include <glew/glew.h>

#include <SDL_mixer.h>

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "HAL/Base/GameLoop.h"
#include "HAL/Graphics/Renderer.h"
#include "HAL/IGame.h"
#include "HAL/Internal/GlContext.h"
#include "HAL/Internal/Sdl.h"
#include "HAL/Internal/SdlWindow.h"

namespace HAL
{
#ifdef CONCURRENT_ACCESS_DETECTION
	ConcurrentAccessDetector gSDLAccessDetector;
	ConcurrentAccessDetector gSDLEventsAccessDetector;
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
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);

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

		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			ReportError("SDL_mixer init failed");
		}
	}

	Engine::~Engine()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		Mix_CloseAudio();
	}

	void Engine::init(IGame* game, InputControllersData* inputData)
	{
		mPimpl->mGame = game;
		mPimpl->mInputDataPtr = inputData;
		mPimpl->mWindow.show();
	}

	void Engine::start()
	{
		mPimpl->start();
	}

	Vector2D Engine::getWindowSize() const
	{
		return Vector2D(static_cast<float>(mPimpl->mWindowWidth), static_cast<float>(mPimpl->mWindowHeight));
	}

	void Engine::releaseRenderContext()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
		SDL_GL_MakeCurrent(nullptr, 0);
	}

	void Engine::acquireRenderContext()
	{
		DETECT_CONCURRENT_ACCESS(gSDLAccessDetector);
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
		DETECT_CONCURRENT_ACCESS(gSDLEventsAccessDetector);
		return mPimpl->mLastFrameEvents;
	}

	void Engine::Impl::start()
	{
		AssertFatal(mGame, "Game should be set to Engine before calling start()");
		RunGameLoop(*mGame, nullptr, [this]{ parseEvents(); }, [this]{
			DETECT_CONCURRENT_ACCESS(gSDLEventsAccessDetector);
			mLastFrameEvents.clear();
		});
	}

	void Engine::Impl::parseEvents()
	{
//		SCOPED_PROFILER("Engine::Impl::parseEvents");
		DETECT_CONCURRENT_ACCESS(gSDLEventsAccessDetector);
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
					mInputDataPtr->controllerStates.keyboardState.updateButtonState(event.key.keysym.scancode, true);
				}
				break;
			case SDL_KEYUP:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates.keyboardState.updateButtonState(event.key.keysym.scancode, false);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates.mouseState.updateButtonState(event.button.button, true);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (mInputDataPtr)
				{
					mInputDataPtr->controllerStates.mouseState.updateButtonState(event.button.button, false);
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
					mInputDataPtr->controllerStates.mouseState.updateAxis(0, mouseRelativePos.x);
					mInputDataPtr->controllerStates.mouseState.updateAxis(1, mouseRelativePos.y);
				}
				break;
			default:
				break;
			}

			mLastFrameEvents.push_back(event);
		}
	}
}
