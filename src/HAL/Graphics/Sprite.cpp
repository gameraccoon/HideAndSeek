#include "Base/precomp.h"

#include "Base/Types/String/Path.h"

#include "HAL/Graphics/Sprite.h"

#include "HAL/Graphics/SdlSurface.h"
#include "HAL/Base/Types.h"
#include "HAL/Base/Engine.h"

#include "Utils/ResourceManagement/ResourceManager.h"

namespace Graphics
{
	struct LoadSpriteData
	{
		ResourceHandle surfaceHandle;
		QuadUV uv;
	};

	static UniqueAny CalculateSpriteDependencies(UniqueAny&& resource, ResourceManager& resourceManager, ResourceHandle handle)
	{
		SCOPED_PROFILER("CalculateSpriteDependencies");

		const ResourcePath* pathPtr = resource.cast<ResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in StartSpriteLoading (expected ResourcePath)");
			return {};
		}

		const ResourcePath& path = *pathPtr;

		std::string surfacePath;
		Graphics::QuadUV spriteUV;
		auto it = resourceManager.getAtlasFrames().find(path);
		if (it != resourceManager.getAtlasFrames().end())
		{
			surfacePath = it->second.atlasPath;
			spriteUV = it->second.quadUV;
		}
		else
		{
			surfacePath = path;
		}
		ResourceHandle originalSurfaceHandle = resourceManager.lockResource<Graphics::Surface>(static_cast<ResourcePath>(surfacePath));
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

	std::string Sprite::GetUniqueId(const std::string& filename)
	{
		return std::string("spr-") + filename;
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
