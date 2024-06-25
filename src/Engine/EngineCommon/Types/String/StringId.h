#pragma once

#include <compare>
#include <string>
#include <unordered_map>

#include "EngineCommon/Types/BasicTypes.h"

#include <fnv1a/hash_fnv1a_constexpr.h>
#include <nlohmann/json_fwd.hpp>

/**
 * String type used for technical identifiers that
 * don't need to be presented to the player in any form
 *
 * Can be fully replaced with integer values in production builds
 */
class StringId
{
public:
	friend struct std::hash<StringId>;
	friend class StringIdManager;

	using KeyType = u64;

public:
	constexpr StringId() = default;

	[[nodiscard]] constexpr bool isValid() const noexcept { return mHash != InvalidValue; }

	// compare only hashes
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const StringId& other) const noexcept = default;

private:
	constexpr explicit StringId(const KeyType hash) noexcept
		: mHash(hash)
	{
	}

private:
	static constexpr KeyType InvalidValue = 0;
	KeyType mHash = InvalidValue;
};

class StringIdManager;
extern StringIdManager gStringIdManagerInstance;

/**
 * A globally-accessible manager that converts strings to StringIds and back.
 *
 * Don't use this class explicitly.
 * For conversations to StringId use STR_TO_ID on your string literals or runtime strings.
 * For getting a string from StringId use ID_TO_STR.
 * For initialization (in main function) use INITIALIZE_STRING_IDS
 */
class StringIdManager
{
public:
	// never call this function explicitly
	// use STR_TO_ID macro as it being parsed by external code preprocessing tools)
	// see scripts/build/generators/generate_string_ids.py
	static consteval StringId StringToId(const char* const stringLiteral) noexcept
	{
		return StringId(hash_64_fnv1a_const(stringLiteral));
	}

	static StringId StringToId(const std::string_view string) noexcept
	{
		return gStringIdManagerInstance.stringToId(string);
	}

	StringId stringToId(const std::string_view stringValue);
	const std::string& getStringFromId(StringId id);
	void initializeStringIds(std::unordered_map<StringId::KeyType, std::string>&& predefinedIds);

private:
	std::unordered_map<StringId::KeyType, std::string> mStringIdsToStringsMap;
};

//fwd, we assume this is implemented somewhere in GameCommon so we can use it in our macro
namespace GameCommon
{
	std::unordered_map<uint64_t, std::string> GetGatheredStringIds() noexcept;
}

#define STR_TO_ID(strLiteral) StringIdManager::StringToId(strLiteral)
#define ID_TO_STR(id) gStringIdManagerInstance.getStringFromId(id)
#define INITIALIZE_STRING_IDS() gStringIdManagerInstance.initializeStringIds(GameCommon::GetGatheredStringIds())

template<>
struct std::hash<StringId>
{
	size_t operator()(const StringId& stringId) const noexcept
	{
		// it's already a unique hash
		return stringId.mHash;
	}
};

inline std::string to_string(const StringId stringId)
{
	return ID_TO_STR(stringId);
}

void to_json(nlohmann::json& outJson, const StringId& stringId);
void from_json(const nlohmann::json& json, StringId& stringId);
