#include "MonsterState.h"

#include "AnimationTypes.h"
#include "CharacterMovementComponent.h"
#include "CombatTypes.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"
#include "Random.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimatorComponent.h"

#include <utility>

namespace gm
{
	namespace
	{
		constexpr float MonsterAirborneLaunchSpeed = 16.f;
	}

	void MonsterState::FaceTarget(MonsterStateContext& context, float deltaTime) const
	{
		if (context.combatComponent == nullptr || context.moveComponent == nullptr)
			return;

		context.moveComponent->FaceDirection(context.combatComponent->GetTargetDirection(), deltaTime);
	}

	void MonsterState::SetRootMotionEnabled(MonsterStateContext& context, bool enabled) const
	{
		if (context.moveComponent != nullptr)
			context.moveComponent->SetRootMotionEnabled(enabled);
	}

	void MonsterState::OnDamaged(MonsterStateContext& context, const HitEvent& event)
	{
		if (event.damageResult.state != DamageState::Applied)
			return;

		MonsterStateId nextStateId = event.damageResult.isDead ? MonsterStateId::Dead : MonsterStateId::Damage;
		if (event.damageResult.isDead == false && event.damage.hitReactionType == HitReactionType::Airborne)
			nextStateId = MonsterStateId::Airborne;

		context.stateMachine->ChangeState(nextStateId, true);
	}

	bool MonsterState::PlayAnimation(MonsterStateContext& context, const std::wstring& clipName, bool isLoop, float blendDuration) const
	{
		GM_ASSERT_RETURN_VAL(context.animatorComponent, false, "Monster State에 SkeletalAnimatorComponent가 연결되지 않았습니다.");

		AnimationPlayOption playOption{};
		playOption.loopOverride = isLoop;
		playOption.blendDuration = blendDuration;
		return context.animatorComponent->Play(clipName, playOption);
	}

	bool MonsterState::IsAnimationCompleted(const MonsterStateContext& context) const
	{
		return context.animatorComponent != nullptr && context.animatorComponent->GetState() == AnimationState::Completed;
	}

	MonsterAirborneState::MonsterAirborneState(std::wstring launchClipName, std::wstring fallClipName, std::array<std::wstring, 3> hitClipNames)
		: _launchClipName(std::move(launchClipName)), _fallClipName(std::move(fallClipName)), _hitClipNames(std::move(hitClipNames))
	{
	}

	void MonsterAirborneState::Enter(MonsterStateContext& context)
	{
		_phase = Phase::Launch;
		SetRootMotionEnabled(context, false);
		PlayAnimation(context, _launchClipName, false);
		Launch(context, MonsterAirborneLaunchSpeed);
	}

	void MonsterAirborneState::Tick(MonsterStateContext& context, float)
	{
		if (_phase == Phase::Fall || IsAnimationCompleted(context) == false)
			return;

		_phase = Phase::Fall;
		PlayAnimation(context, _fallClipName, true);
	}

	void MonsterAirborneState::OnDamaged(MonsterStateContext& context, const HitEvent& event)
	{
		if (event.damageResult.state != DamageState::Applied)
			return;

		if (event.damageResult.isDead)
		{
			MonsterState::OnDamaged(context, event);
			return;
		}

		if (event.damage.hitReactionType == HitReactionType::Airborne)
		{
			context.stateMachine->ChangeState(MonsterStateId::Airborne, true);
			return;
		}

		PlayAirHitAnimation(context);
		if (event.damage.hitReactionType == HitReactionType::Sky)
			Launch(context, MonsterAirborneLaunchSpeed);
	}

	void MonsterAirborneState::OnGroundContact(MonsterStateContext& context, const NavigationGroundContactEvent&)
	{
		context.stateMachine->ChangeState(MonsterStateId::Down);
	}

	void MonsterAirborneState::Launch(MonsterStateContext& context, float speed) const
	{
		if (context.rigidbodyComponent == nullptr)
			return;

		context.rigidbodyComponent->ClearVerticalVelocity();
		context.rigidbodyComponent->AddImpulse(Vector3::Up * context.rigidbodyComponent->GetMass() * speed);
	}

	void MonsterAirborneState::PlayAirHitAnimation(MonsterStateContext& context)
	{
		_phase = Phase::Hit;
		const size_t clipIndex = static_cast<size_t>(Math::RandomInt(0, static_cast<int32>(_hitClipNames.size() - 1)));
		PlayAnimation(context, _hitClipNames[clipIndex], false);
	}

	MonsterDownState::MonsterDownState(std::wstring clipName)
		: _clipName(std::move(clipName))
	{
	}

	void MonsterDownState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		PlayAnimation(context, _clipName, false);
	}

	void MonsterDownState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::WakeUp);
	}

	MonsterWakeUpState::MonsterWakeUpState(std::wstring clipName)
		: _clipName(std::move(clipName))
	{
	}

	void MonsterWakeUpState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		PlayAnimation(context, _clipName, false, 0.f);
	}

	void MonsterWakeUpState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::Idle);
	}
}
