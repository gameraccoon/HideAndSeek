#pragma once

namespace GameplayInput
{
	enum class InputAxis
	{
		MoveHorizontal = 0,
		MoveVertical,
		AimHorizontal,
		AimVertical,

		// add new elements above this line
		Count
	};

	enum class InputKey
	{
		Shoot = 0,
		Sprint,

		// add new elements above this line
		Count
	};

}
