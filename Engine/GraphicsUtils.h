#pragma once

#include "Types.h"

namespace gm
{
	inline constexpr uint32 Align16(uint32 size)
	{
		return ((size + 15) / 16) * 16;
	}
}
