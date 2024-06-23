#include "Base/precomp.h"

#ifndef DISABLE_SDL

#include "HAL/Audio/Sound.h"

#include <SDL_mixer.h>

namespace Audio
{
	Sound::Sound(const AbsoluteResourcePath& path)
		: mSound(Mix_LoadWAV(path.getAbsolutePathStr().c_str()))
	{
	}
	
	Sound::~Sound()
	{
		Mix_FreeChunk(mSound);
	}

	Mix_Chunk* Sound::getRawSound() const
	{
		return mSound;
	}

	bool Sound::isValid() const
	{
		return mSound != nullptr;
	}
}

#endif // !DISABLE_SDL
