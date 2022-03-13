#pragma once

namespace HAL
{
	class IGame
	{
	public:
		virtual ~IGame() = default;
		virtual void dynamicTimePreFrameUpdate(float dt) = 0;
		virtual void fixedTimeUpdate(float dt) = 0;
		virtual void dynamicTimePostFrameUpdate(float dt) = 0;
		virtual void initResources() = 0;
		virtual void setKeyboardKeyState(int key, bool isPressed) = 0;
		virtual void setMouseKeyState(int key, bool isPressed) = 0;
	};
}
