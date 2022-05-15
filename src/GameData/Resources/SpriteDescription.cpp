#include "Base/precomp.h"

#include "GameData/Resources/SpriteDescription.h"

#include <nlohmann/json.hpp>

void to_json(nlohmann::json& outJson, const SpriteDescription& spriteDescription)
{
	outJson = nlohmann::json::object({
		{"params", spriteDescription.params},
		{"path", spriteDescription.path}
	});
}

void from_json(const nlohmann::json& json, SpriteDescription& outSpriteDescription)
{
	json.at("params").get_to(outSpriteDescription.params);
	json.at("path").get_to(outSpriteDescription.path);
}
