#pragma once

#include "MonsterStateContext.h"
#include "MonsterStateTypes.h"

#include <string>

namespace gm
{
	struct HitEvent;

	inline constexpr float MonsterDefaultBlendDuration = 0.15f;

	class MonsterState
	{
	public:
		virtual ~MonsterState() = default;

		virtual MonsterStateId GetStateId() const = 0;
		virtual void Enter(MonsterStateContext& context) {}
		virtual void Tick(MonsterStateContext& context, float deltaTime) {}
		virtual void OnDamaged(MonsterStateContext& context, const HitEvent& event);
		virtual void Exit(MonsterStateContext& context) {}

	protected:
		bool PlayAnimation(MonsterStateContext& context, const std::wstring& clipName, bool isLoop, float blendDuration = MonsterDefaultBlendDuration) const;
		bool IsAnimationCompleted(const MonsterStateContext& context) const;
	};
}
