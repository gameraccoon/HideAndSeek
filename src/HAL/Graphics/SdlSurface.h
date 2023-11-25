#pragma once

#include <string>
#include <memory>

#include "GameData/Resources/Resource.h"

struct SDL_Surface;
class AbsoluteResourcePath;

namespace Graphics
{
	class Surface : public Resource
	{
	public:
		explicit Surface(const AbsoluteResourcePath& filename);

		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&&) = delete;
		Surface& operator=(Surface&&) = delete;

		~Surface() override;

		bool isValid() const override;

		void setTextureId(unsigned int textureId) { mTextureID = textureId; }
		unsigned int getTextureId() { return mTextureID; }

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
