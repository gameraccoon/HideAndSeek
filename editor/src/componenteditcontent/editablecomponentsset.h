#pragma once

#include <unordered_set>

#include "EngineCommon/Types/String/StringId.h"

namespace Editor
{
	std::unordered_set<StringId> getEditableComponents();
}
