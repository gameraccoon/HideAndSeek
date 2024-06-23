#include "Base/precomp.h"

#ifndef DISABLE_SDL

#include <string>

#include "SdlSurface.h"
#include <glew/glew.h>
#include <SDL_image.h>
#include <SDL_surface.h>

#include "Base/Debug/ConcurrentAccessDetector.h"
#include "Base/Types/ComplexTypes/UniqueAny.h"
#include "Base/Types/String/ResourcePath.h"

#include "GameData/Resources/ResourceHandle.h"

#ifdef CONCURRENT_ACCESS_DETECTION
namespace HAL
{
	extern ConcurrentAccessDetector gSDLAccessDetector;
}
#endif

namespace Graphics
{
	static UniqueAny LoadSurfaceInitStep(UniqueAny&& resource, ResourceManager&, ResourceHandle)
	{
		SCOPED_PROFILER("LoadSurfaceInitStep");

		const AbsoluteResourcePath* pathPtr = resource.cast<AbsoluteResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a surface in CreateSurfaceInitStep");
			return {};
		}

		return UniqueAny::Create<std::unique_ptr<Surface>>(std::make_unique<Surface>(*pathPtr));
	}

	static UniqueAny InitSurfaceRenderThread(UniqueAny&& resource, ResourceManager&, ResourceHandle)
	{
		SCOPED_PROFILER("InitSurfaceRenderThread");
		std::unique_ptr<Surface>* surfacePtr = resource.cast<std::unique_ptr<Surface>>();

		if (!surfacePtr)
		{
			ReportFatalError("We got an incorrect type of value when initing a surface in InitSurfaceRenderThread");
			return {};
		}

		Surface* surface = surfacePtr->get();

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
			return {};
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

		return UniqueAny::Create<Resource::Ptr>(std::move(*surfacePtr));
	}

	static UniqueAny DeinitSurfaceRenderThread(UniqueAny&& resource, ResourceManager&, ResourceHandle)
	{
		const std::unique_ptr<Surface>* surfacePtr = resource.cast<std::unique_ptr<Surface>>();

		if (!surfacePtr) {
			ReportFatalError("We got an incorrect type of value when deinitializing a surface");
			return {};
		}

		Surface* surface = surfacePtr->get();
		const unsigned int textureId = surface->getTextureId();
		glDeleteTextures(1, &textureId);
		return {};
	}

	Surface::Surface(const AbsoluteResourcePath& filename)
		: mSurface(IMG_Load(filename.getAbsolutePathStr().c_str()))
		, mTextureID(0)
	{
		AssertFatal(mSurface, "Unable to load texture %s", filename.getAbsolutePath().c_str());
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

	std::string Surface::GetUniqueId(const AbsoluteResourcePath& filename)
	{
		return filename.getAbsolutePathStr();
	}

	Surface::InitSteps Surface::GetInitSteps()
	{
		return {
			InitStep{
				.thread = Thread::Loading,
				.init = &LoadSurfaceInitStep,
			},
			InitStep{
				.thread = Thread::Render,
				.init = &InitSurfaceRenderThread,
			},
		};
	}

	Surface::DeinitSteps Surface::getDeinitSteps() const
	{
		return {
			DeinitStep{
				.thread = Thread::Render,
				.deinit = &DeinitSurfaceRenderThread,
			},
		};
	}
}

#endif // !DISABLE_SDL
