#pragma once

#include "Event.h"

namespace gm
{
	enum class BeatHitInputType
	{
		WeakAttack,
		StrongAttack,
		AnyAttack,
	};

	enum class BeatHitResult
	{
		Success,
		Miss,
		Cancelled,
	};

	struct BeatHitStartedEvent : EventType
	{
		float targetBeat = 0.f;
		float approachDurationBeats = 0.f;
	};

	struct BeatHitResultEvent : EventType
	{
		BeatHitResult	result = BeatHitResult::Miss;
		float			targetBeat = 0.f;
	};
}
