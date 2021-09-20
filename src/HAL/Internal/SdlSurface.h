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

			int getWidth() const;
			int getHeight() const;

			void bind() const;

			bool isValid() const override;

			static std::string getUniqueId(const std::string& filename);

		private:
			SDL_Surface* mSurface;
			unsigned int mTextureID;
		};
	}
}
