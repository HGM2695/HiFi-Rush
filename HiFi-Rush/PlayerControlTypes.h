#pragma once

#include "Types.h"

namespace gm
{
	using PlayerControlMask = uint32;

	namespace PlayerControl
	{
		inline constexpr PlayerControlMask None = 0;
		inline constexpr PlayerControlMask Movement = 1 << 0;
		inline constexpr PlayerControlMask Action = 1 << 1;
		inline constexpr PlayerControlMask Camera = 1 << 2;
		inline constexpr PlayerControlMask All = Movement | Action | Camera;
	}
}
