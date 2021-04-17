#pragma once

#include "Base/Types/String/Path.h"

#include "HAL/Base/Resource.h"

struct FC_Font;
struct SDL_Renderer;

namespace Graphics
{
	class Font : public HAL::Resource
	{
	public:
		Font() = default;

		explicit Font(const ResourcePath& path, int fontSize);

		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;
		Font(Font&&) = delete;
		Font& operator=(Font&&) = delete;

		~Font() override;

		[[nodiscard]] bool isValid() const override;

		[[nodiscard]] FC_Font* getRawFont() const;

	private:
		FC_Font* mFont = nullptr;
	};
}
