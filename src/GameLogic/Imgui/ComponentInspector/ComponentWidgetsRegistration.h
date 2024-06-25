#pragma once

#include <map>
#include <memory>

#include "EngineCommon/Types/String/StringId.h"

class AbstractComponentImguiWidget;

namespace ComponentWidgetsRegistration
{
	void RegisterInspectWidgets(std::map<StringId, std::unique_ptr<AbstractComponentImguiWidget>>& componentInspectWidgets);
}
