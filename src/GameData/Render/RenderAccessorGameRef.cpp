#include "EngineCommon/precomp.h"

#include "GameData/Render/RenderAccessorGameRef.h"

RenderAccessorGameRef::RenderAccessorGameRef(IRenderAccessor& ref, const int gameInstanceIndex)
	: mPtr(&ref)
	, mGameInstanceIndex(gameInstanceIndex)
{}

void RenderAccessorGameRef::submitData(std::unique_ptr<RenderData>&& newData) const
{
	mPtr->submitData(std::move(newData), mGameInstanceIndex);
}
