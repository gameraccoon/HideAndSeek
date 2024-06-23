#include "Base/precomp.h"

#ifdef IMGUI_ENABLED

#include "GameData/AI/TravelPath.h"

#include "GameLogic/Imgui/ComponentInspector/DataInspectorWidgets/TypesInspector.h"

namespace ImguiDataInspection
{
	template<>
	void Inspector(const char* title, TravelPath& data)
	{
		Inspector(title, data.smoothPath);
	}
}

#endif // IMGUI_ENABLED

