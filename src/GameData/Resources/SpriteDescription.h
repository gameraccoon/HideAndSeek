#pragma once

#include <string>

#include "Base/Types/String/Path.h"

#include "GameData/Resources/SpriteParams.h"

struct SpriteDescription
{
	SpriteParams params;
	ResourcePath path;
};

inline bool operator ==(const SpriteDescription& a, const SpriteDescription& b)
{
	return a.params == b.params && a.path == b.path;
}

inline bool operator !=(const SpriteDescription& a, const SpriteDescription& b)
{
	return !(a == b);
}

void to_json(nlohmann::json& outJson, const SpriteDescription& spriteDescription);
void from_json(const nlohmann::json& json, SpriteDescription& outSpriteDescription);
