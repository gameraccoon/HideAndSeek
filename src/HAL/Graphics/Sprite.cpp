#include "Base/precomp.h"

#ifndef DISABLE_SDL

#include "Base/Types/String/ResourcePath.h"

#include "HAL/Base/Types.h"
#include "HAL/Graphics/SdlSurface.h"
#include "HAL/Graphics/Sprite.h"

#include "Utils/ResourceManagement/ResourceManager.h"

namespace Graphics
{
	struct LoadSpriteData
	{
		ResourceHandle surfaceHandle;
		QuadUV uv;
	};

	static UniqueAny CalculateSpriteDependencies(UniqueAny&& resource, ResourceManager& resourceManager, const ResourceHandle handle)
	{
		SCOPED_PROFILER("CalculateSpriteDependencies");

		const RelativeResourcePath* pathPtr = resource.cast<RelativeResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in StartSpriteLoading (expected ResourcePath)");
			return {};
		}

		const RelativeResourcePath& path = *pathPtr;

		RelativeResourcePath surfacePath;
		QuadUV spriteUV;
		const auto it = resourceManager.getAtlasFrames().find(path);
		if (it != resourceManager.getAtlasFrames().end())
		{
			surfacePath = it->second.atlasPath;
			spriteUV = it->second.quadUV;
		}
		else
		{
			surfacePath = path;
		}
		ResourceHandle originalSurfaceHandle = resourceManager.lockResource<Graphics::Surface>(resourceManager.getAbsoluteResourcePath(surfacePath));
		resourceManager.setFirstResourceDependOnSecond(handle, originalSurfaceHandle);
		return UniqueAny::Create<LoadSpriteData>(originalSurfaceHandle, spriteUV);
	}

	static UniqueAny CreateSpriteInitStep(UniqueAny&& resource, ResourceManager& resourceManager, ResourceHandle)
	{
		SCOPED_PROFILER("CreateSpriteInitStep");
		const LoadSpriteData* spriteData = resource.cast<LoadSpriteData>();

		if (!spriteData)
		{
			ReportFatalError("We got an incorrect type of value when initing a sprite in CreateSpriteInitStep (expected LoadSpriteData)");
			return {};
		}

		const Graphics::Surface* surface = resourceManager.tryGetResource<Graphics::Surface>(spriteData->surfaceHandle);
		AssertRelease(surface != nullptr, "The surface should be loaded before loading sprite");

		return UniqueAny::Create<Resource::Ptr>(std::make_unique<Graphics::Sprite>(surface, spriteData->uv));
	}

	Sprite::Sprite(const Surface* surface, QuadUV uv)
		: mSurface(surface)
		, mUV(uv)
	{
	}

	int Sprite::getHeight() const
	{
		return mSurface->getHeight();
	}

	int Sprite::getWidth() const
	{
		return mSurface->getWidth();
	}

	const Surface* Sprite::getSurface() const
	{
		return mSurface;
	}

	bool Sprite::isValid() const
	{
		return mSurface != nullptr;
	}

	std::string Sprite::GetUniqueId(const RelativeResourcePath& filename)
	{
		return std::string("spr-") + filename.getRelativePathStr();
	}

	Resource::InitSteps Sprite::GetInitSteps()
	{
		return {
			InitStep{
				.thread = Thread::Any,
				.init = &CalculateSpriteDependencies,
			},
			InitStep{
				.thread = Thread::Any,
				.init = &CreateSpriteInitStep,
			},
		};
	}

	Resource::DeinitSteps Sprite::getDeinitSteps() const
	{
		return {};
	}
}

#endif // !DISABLE_SDL
