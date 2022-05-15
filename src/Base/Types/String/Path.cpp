#include "Base/precomp.h"

#include "Base/Types/String/Path.h"

#include <nlohmann/json.hpp>

void to_json(nlohmann::json& outJson, const ResourcePath& path)
{
	outJson = nlohmann::json(static_cast<std::string>(path));
}

void from_json(const nlohmann::json& json, ResourcePath& path)
{
	path = static_cast<ResourcePath>(json.get<std::string>());
}
