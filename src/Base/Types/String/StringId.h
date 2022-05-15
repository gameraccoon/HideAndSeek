#pragma once

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

	using KeyType = uint64_t;

public:
	constexpr StringId() = default;

	[[nodiscard]] bool isValid() const noexcept { return mHash != InvalidValue; }

	[[nodiscard]] bool operator ==(const StringId& other) const noexcept
	{
		return mHash == other.mHash;
	}

	[[nodiscard]] bool operator !=(const StringId& other) const noexcept
	{
		return mHash != other.mHash;
	}

	[[nodiscard]] bool operator <(const StringId& other) const noexcept
	{
		return mHash < other.mHash;
	}

	[[nodiscard]] bool operator >(const StringId& other) const noexcept
	{
		return mHash > other.mHash;
	}

	[[nodiscard]] bool operator <=(const StringId& other) const noexcept
	{
		return mHash <= other.mHash;
	}

	[[nodiscard]] bool operator >=(const StringId& other) const noexcept
	{
		return mHash >= other.mHash;
	}

private:
	constexpr explicit StringId(KeyType hash) noexcept
		: mHash(hash)
	{
	}

private:
	static const KeyType InvalidValue = 0;
	KeyType mHash = InvalidValue;
};

/**
 * A singleton that converts strings to StringIds and back.
 *
 * Don't use this class explicitly.
 * For conversations to StringId use STR_TO_ID on your string literals or runtime strings.
 * For getting a string from StringId use ID_TO_STR.
 */
class StringIdManager
{
public:
	StringIdManager();
	static StringIdManager& Instance();

	// don't call these functions explicitly
	// use STR_TO_ID macro as it being parsed by external code preprocessing tools)
	// see hideandseek/scripts/build/generators/generate_string_ids.py
	static constexpr StringId StringToId(const char* const stringLiteral) noexcept
	{
		return StringId(hash_64_fnv1a_const(stringLiteral));
	}

	static StringId StringToId(const std::string& string) noexcept
	{
		return StringIdManager::Instance().stringToId(string);
	}

	StringId stringToId(const std::string& stringValue);
	const std::string& getStringFromId(StringId id);

private:
	std::unordered_map<StringId::KeyType, std::string> mStringIdsToStringsMap;
};

#define STR_TO_ID(strLiteral) StringIdManager::StringToId(strLiteral)
#define ID_TO_STR(id) StringIdManager::Instance().getStringFromId(id)

namespace std
{
	template<> struct hash<StringId>
	{
		size_t operator()(const StringId& stringId) const noexcept
		{
			// it's already a unique hash
			return static_cast<size_t>(stringId.mHash);
		}
	};
}

inline std::string to_string(StringId stringId)
{
	return ID_TO_STR(stringId);
}

void to_json(nlohmann::json& outJson, const StringId& stringId);
void from_json(const nlohmann::json& json, StringId& stringId);
