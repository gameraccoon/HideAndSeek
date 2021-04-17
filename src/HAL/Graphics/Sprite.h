#pragma once

#include "HAL/EngineFwd.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"

namespace Graphics
{
	namespace Internal
	{
		class Surface;
	}

	class Sprite : public HAL::Resource
	{
	public:
		Sprite(const Internal::Surface* surface, QuadUV uv);

		[[nodiscard]] int getHeight() const;
		[[nodiscard]] int getWidth() const;

		[[nodiscard]] const Internal::Surface* getSurface() const;
		[[nodiscard]] QuadUV getUV() const { return mUV; }

		[[nodiscard]] bool isValid() const override;

	private:
		const Internal::Surface* mSurface = nullptr;
		QuadUV mUV;
	};
}
