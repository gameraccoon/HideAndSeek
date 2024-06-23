#pragma once

#ifndef DISABLE_SDL

#include <string>

#include "GameData/Resources/Resource.h"

struct SDL_Surface;
class AbsoluteResourcePath;

namespace Graphics
{
	class Surface final : public Resource
	{
	public:
		explicit Surface(const AbsoluteResourcePath& filename);

		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&&) = delete;
		Surface& operator=(Surface&&) = delete;

		~Surface() override;

		bool isValid() const override;

		void setTextureId(const unsigned int textureId) { mTextureID = textureId; }
		unsigned int getTextureId() const { return mTextureID; }

		int getWidth() const;
		int getHeight() const;

		void bind() const;
		const SDL_Surface* getRawSurface() const { return mSurface; }

		static std::string GetUniqueId(const AbsoluteResourcePath& filename);
		static InitSteps GetInitSteps();
		DeinitSteps getDeinitSteps() const override;

	private:
		SDL_Surface* mSurface;
		unsigned int mTextureID;
	};
}

#endif // !DISABLE_SDL
