#include "Base/precomp.h"

#include "GameData/Render/RenderAccessorGameRef.h"

RenderAccessorGameRef::RenderAccessorGameRef(IRenderAccessor& ref, int gameInstanceIndex)
	: mPtr(&ref)
	, mGameInstanceIndex(gameInstanceIndex)
{}

void RenderAccessorGameRef::submitData(std::unique_ptr<RenderData>&& newData)
{
	mPtr->submitData(std::move(newData), mGameInstanceIndex);
}
