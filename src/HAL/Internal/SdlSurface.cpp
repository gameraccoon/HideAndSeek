#include "Base/precomp.h"

#include "SdlSurface.h"

#include "Base/Debug/ConcurrentAccessDetector.h"

#include <stdexcept>
#include <string>
#include <glew/glew.h>
#include <sdl/SDL.h>
#include <sdl/SDL_surface.h>
#include <sdl/SDL_image.h>

#ifdef CONCURRENT_ACCESS_DETECTION
namespace HAL
{
	extern ConcurrentAccessDetector gSDLAccessDetector;
}
#endif

namespace Graphics
{
	namespace Internal
	{
		static void InitSurfaceRenderThread(std::unique_ptr<HAL::Resource>& resource)
		{
			SCOPED_PROFILER("InitSurfaceRenderThread");
			if (!resource->isValid())
			{
				return;
			}

			Surface* surface = static_cast<Surface*>(resource.get());

			unsigned int textureId;
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			surface->setTextureId(textureId);

			const SDL_Surface* sdlSurface = surface->getRawSurface();

			int mode;
			switch (sdlSurface->format->BytesPerPixel)
			{
			case 4:
				mode = GL_RGBA;
				break;
			case 3:
				mode = GL_RGB;
				break;
			case 1:
				mode = GL_LUMINANCE_ALPHA;
				break;
			default:
				ReportError("Image with unknown channel profile");
				return;
			}

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				mode,
				sdlSurface->w,
				sdlSurface->h,
				0,
				static_cast<GLenum>(mode),
				GL_UNSIGNED_BYTE,
				sdlSurface->pixels
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			return;
		}

		static void DeinitSurfaceRenderThread(std::unique_ptr<HAL::Resource>& resource)
		{
			Surface* surface = static_cast<Surface*>(resource.get());
			unsigned int textureId = surface->getTextureId();
			glDeleteTextures(1, &textureId);
		}

		Surface::Surface(const std::string& filename)
			: mSurface(IMG_Load(filename.c_str()))
			, mTextureID(0)
		{
			AssertFatal(mSurface, "Unable to load texture %s", filename);
		}

		Surface::~Surface()
		{
			SDL_FreeSurface(mSurface);
		}

		int Surface::getWidth() const
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			return mSurface->w;
		}

		int Surface::getHeight() const
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			return mSurface->h;
		}

		void Surface::bind() const
		{
			DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
			glBindTexture(GL_TEXTURE_2D, mTextureID);
		}

		bool Surface::isValid() const
		{
			return mSurface != nullptr;
		}

		std::string Surface::GetUniqueId(const std::string& filename)
		{
			return filename;
		}

		Surface::InitSteps Surface::GetInitSteps()
		{
			return {
				InitStep{
					.thread = Thread::Render,
					.init = &InitSurfaceRenderThread,
				}
			};
		}

		Surface::DeinitSteps Surface::getDeinitSteps() const
		{
			return {
				DeinitStep{
					.thread = Thread::Render,
					.deinit = &DeinitSurfaceRenderThread,
				}
			};
		}
	}
}
