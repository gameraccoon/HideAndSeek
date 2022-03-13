#include "Base/precomp.h"

#include "GameData/Geometry/BoundingBox.h"


BoundingBox BoundingBox::operator+(Vector2D shift) const noexcept
{
	return BoundingBox(minX + shift.x, minY + shift.y, maxX + shift.x, maxY + shift.y);
}

static_assert(std::is_trivially_copyable<BoundingBox>(), "BoundingBox should be trivially copyable");
static_assert(std::is_trivially_destructible<BoundingBox>(), "BoundingBox should be trivially destructable");
