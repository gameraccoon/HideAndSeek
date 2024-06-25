#pragma once

#ifndef DISABLE_SDL

#include "EngineCommon/Types/String/ResourcePath.h"

#include "GameData/Resources/Resource.h"

struct FC_Font;
struct SDL_Renderer;

namespace Graphics
{
	class Font final : public Resource
	{
	public:
		Font() = default;

		explicit Font(const AbsoluteResourcePath& path, int fontSize);

		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;
		Font(Font&&) = delete;
		Font& operator=(Font&&) = delete;

		~Font() override;

		bool isValid() const override;

		static std::string GetUniqueId(const AbsoluteResourcePath& path, int fontSize);

		FC_Font* getRawFont() const;

	private:
		FC_Font* mFont = nullptr;
	};
}

#endif // !DISABLE_SDL
