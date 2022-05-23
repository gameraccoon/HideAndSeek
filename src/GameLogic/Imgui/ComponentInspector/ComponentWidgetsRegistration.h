#pragma once

#include <map>
#include <memory>

#include "Base/Types/String/StringId.h"

class AbstractComponentImguiWidget;

namespace ComponentWidgetsRegistration
{
	void RegisterInspectWidgets(std::map<StringId, std::unique_ptr<AbstractComponentImguiWidget>>& componentInspectWidgets);
}
