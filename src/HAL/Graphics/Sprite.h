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

		int getHeight() const;
		int getWidth() const;

		const Internal::Surface* getSurface() const;
		QuadUV getUV() const { return mUV; }

		bool isValid() const override;

	private:
		const Internal::Surface* mSurface = nullptr;
		QuadUV mUV;
	};
}
