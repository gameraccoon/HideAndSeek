#include "EngineCommon/precomp.h"

#include "EngineCommon/Types/String/LocalizableText.h"

#include <nlohmann/json.hpp>

void to_json(nlohmann::json& outJson, const TextString& text)
{
	outJson = nlohmann::json(static_cast<std::string>(text));
}

void from_json(const nlohmann::json& json, TextString& text)
{
	text = static_cast<TextString>(json.get<std::string>());
}
