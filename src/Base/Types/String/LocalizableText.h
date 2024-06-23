#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

/**
 * Localizable UTF-8 string for UI and messages shown to the player
 */
class TextString : public std::string
{
public:
	using std::string::string;
	explicit TextString(std::string&& val) : std::string(val) {}
	explicit TextString(const std::string& val) : std::string(val) {}
};

template<>
struct std::hash<TextString>
{
	std::size_t operator()(TextString const& path) const noexcept
	{
		return std::hash<std::string>{}(static_cast<std::string>(path));
	}
};

void to_json(nlohmann::json& outJson, const TextString& text);
void from_json(const nlohmann::json& json, TextString& text);
