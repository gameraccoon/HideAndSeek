#pragma once

class WorldHolder;

class ImguiSystemsTimeReportWindow
{
public:
	void update(struct ImguiDebugData& debugData);

	bool isVisible = false;

private:
	float mMaxTotalTimeMs = 10.0f;
	float mMaxSystemTimeMs = 1.0f;
	int mCurrentFrame = 200;
};
