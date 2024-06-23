#pragma once

#ifndef DISABLE_SDL

namespace HAL
{
	namespace Internal
	{
		class SDLInstance
		{
		public:
			explicit SDLInstance(unsigned int flags);

			SDLInstance(const SDLInstance&) = delete;
			SDLInstance operator=(const SDLInstance&) = delete;
			SDLInstance(SDLInstance&&) = delete;
			SDLInstance operator=(SDLInstance&&) = delete;

			~SDLInstance();
		};
	}
}

#endif // !DISABLE_SDL
