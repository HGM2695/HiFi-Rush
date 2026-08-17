#pragma once

namespace gm
{
	enum class MonsterStateId
	{
		None,

		Idle,
		Move,
		Attack,
		Damage,
		Airborne,
		Down,
		WakeUp,
		Dead,

		Count,
	};
}
