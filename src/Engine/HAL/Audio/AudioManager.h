#pragma once

#ifndef DISABLE_SDL

namespace Audio
{
	class Sound;
	class Music;

	namespace AudioManager
	{
		void PlaySound(const Sound& sound);

		// [0, 1]
		void SetSoundVolume(const Sound& sound, float volume);
		float GetSoundVolume(const Sound& sound);

		void PlayMusic(const Music& music);
		void PauseMusic();
		void ResumeMusic();
		void StopMusic();
		bool IsMusicPlaying();
		bool IsMusicPaused();

		// [0, 1]
		void SetMusicVolume(float volume);
		float GetMusicVolume();
	}
}
#endif // !DISABLE_SDL
