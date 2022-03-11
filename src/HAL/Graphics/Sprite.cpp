#include "Base/precomp.h"

#include "Base/Types/String/Path.h"

#include "HAL/Graphics/Sprite.h"
#include "HAL/Base/ResourceManager.h"

#include "HAL/Internal/SdlSurface.h"
#include "HAL/Base/Types.h"
#include "HAL/Base/Engine.h"

namespace Graphics
{
	struct LoadSpriteData
	{
		ResourceHandle surfaceHandle;
		QuadUV uv;
	};

	static UniqueAny CalculateSpriteDependencies(UniqueAny&& resource, HAL::ResourceManager& resourceManager, ResourceHandle handle)
	{
		SCOPED_PROFILER("CalculateSpriteDependencies");

		const ResourcePath* pathPtr = resource.cast<ResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in StartSpriteLoading");
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
		ResourceHandle originalSurfaceHandle = resourceManager.lockResource<Graphics::Internal::Surface>(static_cast<ResourcePath>(surfacePath));
		resourceManager.setFirstResourceDependOnSecond(handle, originalSurfaceHandle);
		return UniqueAny::Create<LoadSpriteData>(originalSurfaceHandle, spriteUV);
	}

	static UniqueAny CreateSpriteInitStep(UniqueAny&& resource, HAL::ResourceManager& resourceManager, ResourceHandle)
	{
		SCOPED_PROFILER("CreateSpriteInitStep");
		const LoadSpriteData* spriteData = resource.cast<LoadSpriteData>();

		if (!spriteData)
		{
			ReportFatalError("We got an incorrect type of value when initing a sprite in CreateSpriteInitStep");
			return {};
		}

		const Graphics::Internal::Surface* surface = resourceManager.tryGetResource<Graphics::Internal::Surface>(spriteData->surfaceHandle);
		AssertRelease(surface != nullptr, "The surface should be loaded before loading sprite");

		return UniqueAny::Create<HAL::Resource::Ptr>(std::make_unique<Graphics::Sprite>(surface, spriteData->uv));
	}

	Sprite::Sprite(const Internal::Surface* surface, QuadUV uv)
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

	const Internal::Surface* Sprite::getSurface() const
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

	HAL::Resource::InitSteps Sprite::GetInitSteps()
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

	HAL::Resource::DeinitSteps Sprite::getDeinitSteps() const
	{
		return {};
	}
}
