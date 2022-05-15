#include "Base/precomp.h"

#include "GameData/AI/TravelPath.h"

#include "GameLogic/Imgui/ComponentInspector/DataInspectorWidgets/TypesInspector.h"

#include <cstring>

namespace ImguiDataInspection
{
	template<>
	void Inspector(const char* title, TravelPath& data)
	{
		Inspector(title, data.smoothPath);
	}
}
