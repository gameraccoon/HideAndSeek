#pragma once

#include "Base/Types/String/Path.h"

#include "HAL/Base/Resource.h"

struct Mix_Chunk;

namespace Audio
{
	class Sound : public HAL::Resource
	{
	public:
		Sound() = default;
		explicit Sound(const ResourcePath& path);

		Sound(const Sound&) = delete;
		Sound& operator=(const Sound&) = delete;
		Sound(Sound&&) = delete;
		Sound& operator=(Sound&&) = delete;

		~Sound() override;

		Mix_Chunk* getRawSound() const;

		bool isValid() const override;

	private:
		Mix_Chunk* mSound = nullptr;
	};
}
