#include "Base/precomp.h"

#include <nlohmann/json.hpp>

#include "Base/Types/String/ResourcePath.h"

void to_json(nlohmann::json& outJson, const RelativeResourcePath& path)
{
	outJson = nlohmann::json(path.getRelativePathStr());
}

void from_json(const nlohmann::json& json, RelativeResourcePath& path)
{
	path = RelativeResourcePath(json.get<std::string>());
}
