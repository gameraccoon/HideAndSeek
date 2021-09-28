#pragma once

#include <string>
#include <memory>

#include "HAL/Base/Resource.h"

struct SDL_Surface;

namespace Graphics
{
	namespace Internal
	{
		class Surface : public HAL::Resource
		{
		public:
			explicit Surface(const std::string& filename);

			Surface(const Surface&) = delete;
			Surface& operator=(const Surface&) = delete;
			Surface(Surface&&) = delete;
			Surface& operator=(Surface&&) = delete;

			~Surface() override;

			bool isValid() const override;

			const SpecialThreadInit* getSpecialThreadInitialization() const override;

			void setTextureId(unsigned int textureId) { mTextureID = textureId; }
			unsigned int getTextureId() { return mTextureID; }

			int getWidth() const;
			int getHeight() const;

			void bind() const;
			const SDL_Surface* getRawSurface() const { return mSurface; }

			static std::string GetUniqueId(const std::string& filename);

		private:
			SDL_Surface* mSurface;
			unsigned int mTextureID;
			static SpecialThreadInit RenderThreadInit;
		};
	}
}
