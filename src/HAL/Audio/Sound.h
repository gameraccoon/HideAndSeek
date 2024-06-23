#pragma once

#ifndef DISABLE_SDL

#include "Base/Types/String/ResourcePath.h"

#include "GameData/Resources/Resource.h"

struct Mix_Chunk;

namespace Audio
{
	class Sound final : public Resource
	{
	public:
		Sound() = default;
		explicit Sound(const AbsoluteResourcePath& path);

		Sound(const Sound&) = delete;
		Sound& operator=(const Sound&) = delete;
		Sound(Sound&&) = delete;
		Sound& operator=(Sound&&) = delete;

		~Sound() override;

		Mix_Chunk* getRawSound() const;

		[[nodiscard]] bool isValid() const override;

	private:
		Mix_Chunk* mSound = nullptr;
	};
}
#endif // DISABLE_SDL
