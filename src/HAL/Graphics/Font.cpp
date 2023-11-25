#include "Base/precomp.h"

#include "HAL/Graphics/Font.h"

#include "HAL/Base/Engine.h"

//#include "SDL_FontCache/SDL_FontCache.h"

namespace Graphics
{
	Font::Font(const AbsoluteResourcePath& /*path*/, int /*fontSize*/)
	{
		//mFont = FC_CreateFont();
		//FC_LoadFont(mFont, renderer, path.c_str(), fontSize, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
	}

	Font::~Font()
	{
		//FC_FreeFont(mFont);
	}

	bool Font::isValid() const
	{
		return mFont != nullptr;
	}

	std::string Font::GetUniqueId(const AbsoluteResourcePath& path, int fontSize)
	{
		return FormatString("%s:%d", path.getAbsolutePath().c_str(), fontSize);
	}

	FC_Font* Font::getRawFont() const
	{
		return mFont;
	}
}
