#include "Base/precomp.h"

#include "GlContext.h"

#include "SdlWindow.h"
#include <stdexcept>
#include <string>

#include "Base/Debug/ConcurrentAccessDetector.h"

namespace HAL
{
#ifdef CONCURRENT_ACCESS_DETECTION
	extern ConcurrentAccessDetector gSDLAccessDetector;
#endif

	namespace Internal
	{
		GlContext::GlContext(Window& window)
			: mContext(SDL_GL_CreateContext(window.getRawWindow()))
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			const char* error = SDL_GetError();
			if (*error != '\0')
			{
				throw std::runtime_error(std::string("Error initialising OpenGL context: ") + error);
			}
		}

		GlContext::~GlContext()
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			SDL_GL_DeleteContext(mContext);
		}

		SDL_GLContext GlContext::getRawGLContext()
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			return mContext;
		}
	}
}
