#pragma once

#include "EngineData/Geometry/Vector2D.h"

struct SpriteParams
{
	Vector2D size = ZERO_VECTOR;
	Vector2D anchor = Vector2D(0.5f, 0.5f);
};

inline bool operator ==(const SpriteParams& a, const SpriteParams& b)
{
	return a.size == b.size && a.anchor == b.anchor;
}

inline bool operator !=(const SpriteParams& a, const SpriteParams& b)
{
	return !(a == b);
}

void to_json(nlohmann::json& outJson, const SpriteParams& spriteParams);
void from_json(const nlohmann::json& json, SpriteParams& outSpriteParams);
