#include "Base/precomp.h"

#include "HAL/Audio/Sound.h"

#include <SDL_mixer.h>

#include "HAL/Base/Engine.h"

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
