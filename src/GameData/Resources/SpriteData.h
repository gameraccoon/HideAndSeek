#pragma once

#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Resources/SpriteParams.h"

struct SpriteData
{
	SpriteData() = default;
	SpriteData(SpriteParams params, ResourceHandle spriteHandle)
		: params(params)
		, spriteHandle(spriteHandle)
	{}

	SpriteParams params;
	ResourceHandle spriteHandle;
};
