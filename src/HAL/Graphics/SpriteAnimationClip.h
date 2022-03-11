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

		bool isValid() const override;

		const ResourceHandle& getSprite(float progress) const;
		const std::vector<ResourceHandle>& getSprites() const;

		static std::string GetUniqueId(const std::string& filename);
		static InitSteps GetInitSteps();
		DeinitSteps getDeinitSteps() const override;

	private:
		std::vector<ResourceHandle> mSprites;
	};
}
