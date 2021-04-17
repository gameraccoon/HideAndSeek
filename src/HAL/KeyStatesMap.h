#pragma once

#include <unordered_set>
#include <vector>

namespace HAL
{
	class KeyStatesMap
	{
	public:
		void updateState(int key, bool isPressed);
		void clearLastFrameState();
		[[nodiscard]] bool isPressed(int key) const;
		[[nodiscard]] bool isJustPressed(int key) const;
		[[nodiscard]] bool isJustReleased(int key) const;

	private:
		std::unordered_set<int> mPressedKeys;
		std::vector<int> mLastFramePressedKeys;
		std::vector<int> mLastFrameReleasedKeys;
	};
}
