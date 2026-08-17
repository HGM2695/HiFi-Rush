#pragma once

#include "Types.h"

namespace gm
{
	enum class RhythmRank
	{
		C,
		B,
		A,
		S,

		Count
	};

	inline constexpr uint32 RhythmRankCount = static_cast<uint32>(RhythmRank::Count);

	inline uint32 GetRhythmRankIndex(RhythmRank rank)
	{
		return static_cast<uint32>(rank);
	}
}
