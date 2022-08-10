#pragma once

#ifdef IMGUI_ENABLED

#include <vector>

#include "imgui.h"

#include "Base/Types/String/StringHelpers.h"

struct Vector2D;
class Rotator;
class StringId;

namespace ImguiDataInspection
{
	template<typename T>
	void Inspector(const char* title, T& /*data*/)
	{
		ImGui::Text("Type inspection for '%s' is not yet implemented", title);
	}

	template<>
	void Inspector(const char* title, bool& data);

	template<>
	void Inspector(const char* title, int& data);

	template<>
	void Inspector(const char* title, float& data);

	template<>
	void Inspector(const char* title, Vector2D& data);

	template<>
	void Inspector(const char* title, Rotator& data);

	template<>
	void Inspector(const char* title, StringId& data);

	template<typename T>
	void Inspector(const char* title, std::vector<T>& data)
	{
		if (ImGui::TreeNode(title))
		{
			for (size_t i = 0, iSize = data.size(); i < iSize; ++i)
			{
				Inspector(FormatString("%s[%u]", title, i).c_str(), data[i]);
			}
			ImGui::TreePop();
			ImGui::Separator();
		}
	}
}

#endif // IMGUI_ENABLED
