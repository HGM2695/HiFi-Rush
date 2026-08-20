#pragma once

#include "Event.h"

namespace gm
{
	enum class QamilStateId
	{
		None,

		Inactive,
		Idle,
		Move,
		NormalAttack,
		Sweep,
		Stump,
		Missile,
		Laser,
		Chain,
		Dead,

		Count,
	};

	enum class QamilPhase
	{
		Phase1,
		Phase2,
		Phase3,

		Count,
	};

	struct QamilPhaseChangedEvent final : EventType
	{
		QamilPhase previousPhase = QamilPhase::Phase1;
		QamilPhase currentPhase = QamilPhase::Phase1;
	};

	struct QamilDefeatedEvent final : EventType
	{
	};
}
