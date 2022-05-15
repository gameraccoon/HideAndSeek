#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

/**
 * String used for local path to resources
 */
class ResourcePath : public std::string
{
public:
	using std::string::string;
	explicit ResourcePath(std::string&& val) : std::string(std::move(val)) {}
	explicit ResourcePath(const std::string& val) : std::string(val) {}
};

namespace std
{
	template<> struct hash<ResourcePath>
	{
		size_t operator()(ResourcePath const& path) const noexcept
		{
			return hash<string>{}(static_cast<string>(path));
		}
	};
}

void to_json(nlohmann::json& outJson, const ResourcePath& path);
void from_json(const nlohmann::json& json, ResourcePath& path);
