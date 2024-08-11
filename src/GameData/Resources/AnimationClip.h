#pragma once

#include <vector>

#include "GameData/Resources/AnimationClipParams.h"
#include "GameData/Resources/ResourceHandle.h"

struct AnimationClip
{
	std::vector<ResourceHandle> sprites;
	AnimationClipParams params;
	float progress = 0.0f;
	int spriteId;
	ResourceHandle animation;
};
