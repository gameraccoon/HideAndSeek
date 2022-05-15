#include "Base/precomp.h"

#include "Base/Types/String/StringId.h"

#include <nlohmann/json.hpp>

#include "Base/Types/String/GatheredStringIds.generated.h"

void to_json(nlohmann::json& outJson, const StringId& stringId)
{
	outJson = nlohmann::json(ID_TO_STR(stringId));
}

void from_json(const nlohmann::json& json, StringId& stringId)
{
	stringId = STR_TO_ID(json.get<std::string>());
}

StringIdManager::StringIdManager()
	: mStringIdsToStringsMap(GetGatheredStringIds())
{
}

StringIdManager& StringIdManager::Instance()
{
	static StringIdManager instance;
	return instance;
}

StringId StringIdManager::stringToId(const std::string& stringValue)
{
	const StringId::KeyType hash = hash_64_fnv1a_const(stringValue.c_str());
	AssertFatal(hash != 0UL, "String hashing result should not be 0: '%s'", stringValue);
	MAYBE_UNUSED auto [it, hasInserted] = mStringIdsToStringsMap.emplace(hash, stringValue);
	AssertFatal(hasInserted || it->second == stringValue, "Hash collision for '%s' and '%s'", stringValue.c_str(), it->second.c_str());
	return StringId(hash);
}

const std::string& StringIdManager::getStringFromId(StringId id)
{
	const static std::string emptyString;

	auto it = mStringIdsToStringsMap.find(id.mHash);
	if (it != mStringIdsToStringsMap.end())
	{
		return it->second;
	}
	else
	{
		ReportError("String representation for id '%d' not found", id.mHash);
		return emptyString;
	}
}

static_assert(sizeof(StringId) == sizeof(uint64_t), "StringId is too big");
static_assert(std::is_trivially_copyable<StringId>(), "StringId should be trivially copyable");
