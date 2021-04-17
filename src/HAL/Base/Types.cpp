#include "Base/precomp.h"

#include "HAL/Base/Types.h"

#include <type_traits>

namespace Graphics
{
	static_assert(std::is_trivially_copyable<QuadUV>(), "QuadUV should be trivially copyable");
	static_assert(std::is_trivially_destructible<QuadUV>(), "QuadUV should be trivially destructible");
}
