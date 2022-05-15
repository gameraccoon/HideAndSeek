#pragma once

class AbstractComponentImguiWidget
{
public:
	virtual ~AbstractComponentImguiWidget() = default;

	virtual void update(void* component) = 0;
};
