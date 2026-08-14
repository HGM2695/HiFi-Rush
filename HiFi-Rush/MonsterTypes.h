#pragma once

#include "MathTypes.h"
#include "Types.h"

namespace gm
{
	enum class MonsterType : uint32
	{
		Sjango,
		Sword,
		Gunner,

		Count,
	};

	struct MonsterSpawnData
	{
		MonsterType	type = MonsterType::Count;
		Matrix		world = Matrix::Identity;
	};
}
