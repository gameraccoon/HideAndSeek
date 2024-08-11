#include "EngineCommon/precomp.h"

#ifdef IMGUI_ENABLED

#include <cstring>

#include "EngineCommon/Types/String/StringId.h"

#include "EngineData/Geometry/Vector2D.h"

#include "GameLogic/Imgui/ComponentInspector/DataInspectorWidgets/BasicTypesInspector.h"

namespace ImguiDataInspection
{
	template<>
	void Inspector(const char* title, bool& data)
	{
		ImGui::Checkbox(title, &data);
	}

	template<>
	void Inspector(const char* title, int& data)
	{
		ImGui::InputInt(title, &data);
	}

	template<>
	void Inspector(const char* title, float& data)
	{
		ImGui::InputFloat(title, &data);
	}

	template<>
	void Inspector(const char* title, Vector2D& data)
	{
		if (ImGui::TreeNode(title))
		{
			ImGui::InputFloat("x", &data.x, 1.0f, 20.0f);
			ImGui::InputFloat("y", &data.y, 1.0f, 20.0f);
			ImGui::TreePop();
			ImGui::Separator();
		}
	}

	template<>
	void Inspector(const char* title, Rotator& data)
	{
		float value = data.getValue();
		if (ImGui::InputFloat(title, &value))
		{
			data = Rotator(value);
		}
	}

	template<>
	void Inspector(const char* title, StringId& data)
	{
		char buffer[256];
		std::strcpy(buffer, ID_TO_STR(data).c_str());
		if (ImGui::InputText(title, buffer, sizeof(buffer)))
		{
			data = STR_TO_ID(std::string(buffer));
		}
	}
} // namespace ImguiDataInspection

#endif // IMGUI_ENABLED
