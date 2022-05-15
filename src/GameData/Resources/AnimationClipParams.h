#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

struct AnimationClipParams
{
	float speed = 1.0f;
	bool isLooped = false;
};

inline bool operator ==(const AnimationClipParams& a, const AnimationClipParams& b)
{
	return a.speed == b.speed && a.isLooped == b.isLooped;
}

inline bool operator !=(const AnimationClipParams& a, const AnimationClipParams& b)
{
	return !(a == b);
}

void to_json(nlohmann::json& outJson, const AnimationClipParams& animationParams);
void from_json(const nlohmann::json& json, AnimationClipParams& outAnimationParams);
