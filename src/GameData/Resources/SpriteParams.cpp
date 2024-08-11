#include "EngineCommon/precomp.h"

#include "GameData/Resources/SpriteParams.h"

#include <nlohmann/json.hpp>

void to_json(nlohmann::json& outJson, const SpriteParams& spriteParams)
{
	outJson = nlohmann::json::object({
		{ "size", spriteParams.size },
		{ "anchor", spriteParams.anchor },
	});
}

void from_json(const nlohmann::json& json, SpriteParams& outSpriteParams)
{
	json.at("size").get_to(outSpriteParams.size);
	json.at("anchor").get_to(outSpriteParams.anchor);
}
