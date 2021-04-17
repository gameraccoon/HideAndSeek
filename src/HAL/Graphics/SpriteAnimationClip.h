#pragma once

#include <memory>

#include "HAL/EngineFwd.h"

#include <vector>

#include "GameData/Core/ResourceHandle.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"

#include "HAL/Graphics/Sprite.h"

namespace Graphics
{
	class SpriteAnimationClip : public HAL::Resource
	{
	public:
		SpriteAnimationClip() = default;
		explicit SpriteAnimationClip(std::vector<ResourceHandle>&& sprites);

		[[nodiscard]] bool isValid() const override;

		[[nodiscard]] const ResourceHandle& getSprite(float progress) const;
		[[nodiscard]] const std::vector<ResourceHandle>& getSprites() const;

	private:
		std::vector<ResourceHandle> mSprites;
	};
}
