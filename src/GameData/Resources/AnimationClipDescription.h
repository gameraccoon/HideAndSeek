#pragma once

#include <nlohmann/json_fwd.hpp>

#include "EngineCommon/Types/String/ResourcePath.h"

#include "GameData/Resources/AnimationClipParams.h"
#include "GameData/Resources/SpriteParams.h"

struct AnimationClipDescription
{
	AnimationClipParams params;
	SpriteParams spriteParams;
	RelativeResourcePath path;
};

inline bool operator==(const AnimationClipDescription& a, const AnimationClipDescription& b)
{
	return a.params == b.params && a.spriteParams == b.spriteParams && a.path == b.path;
}

inline bool operator!=(const AnimationClipDescription& a, const AnimationClipDescription& b)
{
	return !(a == b);
}

void to_json(nlohmann::json& outJson, const AnimationClipDescription& animationDescription);
void from_json(const nlohmann::json& json, AnimationClipDescription& outAnimationDescription);
