#pragma once

#include <vector>

#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Resources/AnimationClipParams.h"

struct AnimationClip
{
	std::vector<ResourceHandle> sprites;
	AnimationClipParams params;
	float progress = 0.0f;
	int spriteId;
	ResourceHandle animation;
};
