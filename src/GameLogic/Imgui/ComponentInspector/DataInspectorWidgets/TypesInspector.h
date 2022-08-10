#pragma once

#ifdef IMGUI_ENABLED

#include "GameLogic/Imgui/ComponentInspector/DataInspectorWidgets/BasicTypesInspector.h"

class TravelPath;

namespace ImguiDataInspection
{
	template<>
	void Inspector(const char* title, TravelPath& data);
}

#endif // IMGUI_ENABLED
