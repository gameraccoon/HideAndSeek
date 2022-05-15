#pragma once

#include "GameLogic/Imgui/ComponentInspector/DataInspectorWidgets/BasicTypesInspector.h"

class TravelPath;

namespace ImguiDataInspection
{
	template<>
	void Inspector(const char* title, TravelPath& data);
}
