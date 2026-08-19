#include "MonsterState.h"

#include "AnimationTypes.h"
#include "CharacterMovementComponent.h"
#include "CombatTypes.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"
#include "Random.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"

#include <utility>

namespace gm
{
	namespace
	{
		constexpr float MonsterLaunchRootMotionYWeight = 1.f;
		constexpr float MonsterAirHitRootMotionYWeight = 0.6f;
		constexpr float MonsterAirHitPlayRate = 0.5f;
		constexpr float MonsterAirHitFallTransitionRatio = 0.7f;
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
		SaveMotionSettings(context);
		_phase = Phase::Launch;
		_isSkyHit = false;

		if (context.rigidbodyComponent)
		{
			context.rigidbodyComponent->ClearVerticalVelocity();
			context.rigidbodyComponent->SetUseGravity(false);
		}

		SetRootMotion(context, true, Vector3{ 0.f, MonsterLaunchRootMotionYWeight, 0.f });
		context.animatorComponent->SetPlayRate(1.f);
		PlayAnimation(context, _launchClipName, false);
	}

	void MonsterAirborneState::Tick(MonsterStateContext& context, float)
	{
		if (_phase == Phase::Fall)
			return;

		if ((_phase == Phase::Hit && _isSkyHit && HasReachedSkyHitFallTransition(context)) || IsAnimationCompleted(context))
			BeginFall(context);
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

		PlayAirHitAnimation(context, event.damage.hitReactionType == HitReactionType::Sky);
	}

	void MonsterAirborneState::OnGroundContact(MonsterStateContext& context)
	{
		context.stateMachine->ChangeState(MonsterStateId::Down);
	}

	void MonsterAirborneState::Exit(MonsterStateContext& context)
	{
		if (context.animatorComponent)
			context.animatorComponent->SetPlayRate(1.f);
		RestoreMotionSettings(context);
	}

	void MonsterAirborneState::SaveMotionSettings(MonsterStateContext& context)
	{
		if (context.moveComponent)
		{
			_previousRootMotionEnabled = context.moveComponent->IsRootMotionEnabled();
			_previousRootMotionWeight = context.moveComponent->GetRootMotionWeight();
		}
		if (context.rigidbodyComponent)
			_previousUseGravity = context.rigidbodyComponent->IsUseGravity();

		_hasSavedMotionSettings = true;
	}

	void MonsterAirborneState::RestoreMotionSettings(MonsterStateContext& context)
	{
		if (_hasSavedMotionSettings == false)
			return;

		if (context.moveComponent)
		{
			context.moveComponent->SetRootMotionWeight(_previousRootMotionWeight);
			context.moveComponent->SetRootMotionEnabled(_previousRootMotionEnabled);
		}
		if (context.rigidbodyComponent)
			context.rigidbodyComponent->SetUseGravity(_previousUseGravity);

		_hasSavedMotionSettings = false;
	}

	void MonsterAirborneState::SetRootMotion(MonsterStateContext& context, bool enabled, const Vector3& weight) const
	{
		if (context.moveComponent == nullptr)
			return;

		context.moveComponent->SetRootMotionWeight(weight);
		context.moveComponent->SetRootMotionEnabled(enabled);
	}

	void MonsterAirborneState::PlayAirHitAnimation(MonsterStateContext& context, bool applyRootMotionY)
	{
		_phase = Phase::Hit;
		_isSkyHit = applyRootMotionY;
		const float rootMotionYWeight = applyRootMotionY ? MonsterAirHitRootMotionYWeight : 0.f;
		SetRootMotion(context, true, Vector3{ 0.f, rootMotionYWeight, 0.f });

		if (context.rigidbodyComponent)
		{
			context.rigidbodyComponent->ClearVerticalVelocity();
			context.rigidbodyComponent->SetUseGravity(applyRootMotionY == false);
		}

		const size_t clipIndex = static_cast<size_t>(Math::RandomInt(0, static_cast<int32>(_hitClipNames.size() - 1)));
		PlayAnimation(context, _hitClipNames[clipIndex], false);

		const float playRate = applyRootMotionY ? MonsterAirHitPlayRate : 1.f;
		context.animatorComponent->SetPlayRate(playRate);
	}

	bool MonsterAirborneState::HasReachedSkyHitFallTransition(const MonsterStateContext& context) const
	{
		if (context.animatorComponent == nullptr)
			return false;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr || clip->GetLength() <= 0.f)
			return false;

		return context.animatorComponent->GetPlayTime() / clip->GetLength() >= MonsterAirHitFallTransitionRatio;
	}

	void MonsterAirborneState::BeginFall(MonsterStateContext& context)
	{
		_phase = Phase::Fall;
		_isSkyHit = false;
		SetRootMotion(context, false, Vector3{});
		if (context.rigidbodyComponent)
			context.rigidbodyComponent->SetUseGravity(true);
		context.animatorComponent->SetPlayRate(1.f);
		PlayAnimation(context, _fallClipName, true);
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
