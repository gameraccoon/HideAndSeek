#include "EngineCommon/precomp.h"

#include "EngineCommon/Types/String/StringId.h"

#include <algorithm>
#include <inttypes.h>

#include <fnv1a/hash_fnv1a.h>
#include <nlohmann/json.hpp>

StringIdManager gStringIdManagerInstance;

StringId StringIdManager::stringToId(const std::string_view stringValue)
{
	const StringId::KeyType hash = hash_64_fnv1a(stringValue.data(), stringValue.size());
	AssertFatal(hash != 0UL, "String hashing result should not be 0: '%s'", stringValue);
	MAYBE_UNUSED auto [it, hasInserted] = mStringIdsToStringsMap.emplace(hash, stringValue);
	AssertFatal(hasInserted || it->second == stringValue, "Hash collision for '%s' and '%s'", stringValue, it->second.c_str());
	return StringId(hash);
}

const std::string& StringIdManager::getStringFromId(const StringId id)
{
	const static std::string emptyString;

	if (const auto it = mStringIdsToStringsMap.find(id.mHash); it != mStringIdsToStringsMap.end())
	{
		return it->second;
	}

	ReportError("String representation for id '%" PRIu64 "' not found", id.mHash);
	return emptyString;
}

void StringIdManager::initializeStringIds(std::unordered_map<StringId::KeyType, std::string>&& predefinedIds)
{
	if (!mStringIdsToStringsMap.empty()) [[unlikely]]
	{
		std::string erroneousValues;
		for (auto& [key, value] : mStringIdsToStringsMap)
		{
			erroneousValues += "'" + value += "', ";
		}
		ReportFatalError("We have some values in StringManager before we initialize it. This means we tried to access it before (possibly from some static initialization, please make sure we don't access the manager before it is initialized). %u values:(%s)", mStringIdsToStringsMap.size(), erroneousValues);
	}

	mStringIdsToStringsMap = std::move(predefinedIds);
}

void to_json(nlohmann::json& outJson, const StringId& stringId)
{
	outJson = nlohmann::json(ID_TO_STR(stringId));
}

void from_json(const nlohmann::json& json, StringId& stringId)
{
	stringId = STR_TO_ID(json.get<std::string>());
}

static_assert(sizeof(StringId) == sizeof(uint64_t), "StringId is too big");
static_assert(std::is_trivially_copyable<StringId>(), "StringId should be trivially copyable");
