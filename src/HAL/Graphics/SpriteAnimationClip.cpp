#include "Base/precomp.h"

#include "HAL/Graphics/SpriteAnimationClip.h"

#include <algorithm>

#include "../Internal/SdlSurface.h"
#include "HAL/Base/Engine.h"

namespace Graphics
{
	SpriteAnimationClip::SpriteAnimationClip(std::vector<ResourceHandle>&& sprites)
		: mSprites(std::move(sprites))
	{
		addDependencies(sprites);
	}

	bool SpriteAnimationClip::isValid() const
	{
		return !mSprites.empty();
	}

	const ResourceHandle& SpriteAnimationClip::getSprite(float progress) const
	{
		return mSprites[std::min(mSprites.size()*static_cast<size_t>(progress), mSprites.size() - 1)];
	}

	const std::vector<ResourceHandle> &SpriteAnimationClip::getSprites() const
	{
		return mSprites;
	}
}
