#pragma once

#include <memory>

struct RenderData;

class IRenderAccessor
{
public:
	virtual void submitData(std::unique_ptr<RenderData>&& newData, int gameInstanceIndex) = 0;
	virtual ~IRenderAccessor() {}
};

class RenderAccessorGameRef
{
public:
	RenderAccessorGameRef(IRenderAccessor& ref, int gameInstanceIndex);

	void submitData(std::unique_ptr<RenderData>&& newData) const;

private:
	// pointer to make the class assignable
	IRenderAccessor* mPtr;
	int mGameInstanceIndex;
};
