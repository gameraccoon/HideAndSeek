#include "Base/precomp.h"

#include "HAL/Audio/Music.h"

#include <SDL_mixer.h>

#include "HAL/Base/Engine.h"

namespace Audio
{
	Music::Music(const ResourcePath& path)
		: mMusic(Mix_LoadMUS(path.c_str()))
	{
	}

	Music::~Music()
	{
		Mix_FreeMusic(mMusic);
	}

	Mix_Music* Music::getRawMusic() const
	{
		return mMusic;
	}

	bool Music::isValid() const
	{
		return mMusic != nullptr;
	}
}
