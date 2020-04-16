#include "Base/precomp.h"

#include "GameLogic/Imgui/ImguiSystemsTimeReportWindow.h"

#include "imgui/imgui.h"

#include "GameData/GameData.h"
#include "GameData/Components/RenderModeComponent.generated.h"

#include "GameLogic/Imgui/ImguiDebugData.h"
#include "Utils/Profiling/SystemFrameRecords.h"

static float TotalTimePlotValueGetter(void* data, int idx)
{
	return static_cast<float>(static_cast<SystemsFrameTime*>(data)[idx].frameTime.count()) * 0.001f;
}

static float SystemTimePlotValueGetter(void* data, int idx)
{
	return static_cast<float>(static_cast<std::chrono::microseconds*>(data)[idx].count()) * 0.001f;
}

void ImguiSystemsTimeReportWindow::update(ImguiDebugData& debugData)
{
	if (isVisible)
	{
		std::vector<SystemsFrameTime>& records = debugData.systemRecords.getFramesRef();

		if (records.empty())
		{
			return;
		}

		char overlay[32];

		ImGui::Begin("Systems Time Report", &isVisible);

		// Total Time plot
		mMaxTotalTimeMs = std::max(mMaxTotalTimeMs, static_cast<float>(std::max_element(records.begin(), records.end(), [](auto& a, auto& b)
		{
			return a.frameTime < b.frameTime;
		})->frameTime.count()) * 0.001f);
		sprintf(overlay, "peak %.2f ms", static_cast<double>(mMaxTotalTimeMs));
		ImGui::PlotLines(
			"Total Time",
			&TotalTimePlotValueGetter,
			static_cast<void*>(records.data()),
			static_cast<int>(records.size()),
			0,
			overlay,
			0,
			mMaxTotalTimeMs,
			ImVec2(0, 100)
		);

		// Frame time by systems plot
		SystemsFrameTime& frame = records.back();
		mMaxSystemTimeMs = std::max(mMaxSystemTimeMs, static_cast<float>(std::max_element(frame.systemsTime.begin(), frame.systemsTime.end())->count()) * 0.001f);
		sprintf(overlay, "peak %.2f ms", static_cast<double>(mMaxSystemTimeMs));
		ImGui::PlotHistogram(
			"Frame Time By Systems",
			&SystemTimePlotValueGetter,
			static_cast<void*>(frame.systemsTime.data()),
			static_cast<int>(frame.systemsTime.size()),
			0,
			overlay,
			0,
			mMaxSystemTimeMs,
			ImVec2(0, 100)
		);

		for (unsigned int i = 0; i < frame.systemsTime.size(); ++i)
		{
			ImGui::Text("%u %s", i, debugData.systemNames[i].c_str());
		}

		ImGui::End();
	}
}
