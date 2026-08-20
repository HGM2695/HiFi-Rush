#include "ChiStateMachineComponent.h"
#include "Application.h"
#include "BeatSystem.h"
#include "ChiAttackState.h"
#include "ChiDamageState.h"
#include "ChiDashState.h"
#include "ChiIdleRunState.h"
#include "ChiJumpState.h"
#include "ChiSpecialState.h"
#include "GameObject.h"
#include "ChiMoveComponent.h"
#include "HealthComponent.h"
#include "HiFiRushStatics.h"
#include "HitBoxComponent.h"
#include "Input.h"
#include "NavMeshControllerComponent.h"
#include "PlayerTargetingComponent.h"
#include "Rigidbody3DComponent.h"
#include "ReverbComponent.h"
#include "SkeletalAnimatorComponent.h"

#include <cmath>

namespace gm
{
	ChiStateMachineComponent::ChiStateMachineComponent(HitBoxComponent* weaponHitBox)
		: _weaponHitBox(weaponHitBox)
	{
	}
	ChiStateMachineComponent::~ChiStateMachineComponent() = default;

	void ChiStateMachineComponent::OnInitialize()
	{
		_animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "ChiStateMachineComponent는 SkeletalAnimatorComponent가 필요합니다.");

		_moveComponent = GetOwner().GetComponent<ChiMoveComponent>();
		GM_ASSERT_RETURN(_moveComponent, "ChiStateMachineComponent는 ChiMoveComponent가 필요합니다.");

		_targetingComponent = GetOwner().GetComponent<PlayerTargetingComponent>();
		GM_ASSERT_RETURN(_targetingComponent, "ChiStateMachineComponent는 PlayerTargetingComponent가 필요합니다.");

		NavMeshControllerComponent* navMeshControllerComponent = GetOwner().GetComponent<NavMeshControllerComponent>();
		GM_ASSERT_RETURN(navMeshControllerComponent, "ChiStateMachineComponent는 NavMeshControllerComponent가 필요합니다.");

		Rigidbody3DComponent* rigidbodyComponent = GetOwner().GetRigidbody3D();
		GM_ASSERT_RETURN(rigidbodyComponent, "ChiStateMachineComponent는 Rigidbody3DComponent가 필요합니다.");

		_healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_healthComponent, "ChiStateMachineComponent는 HealthComponent가 필요합니다.");
		_reverbComponent = GetOwner().GetComponent<ReverbComponent>();
		GM_ASSERT_RETURN(_reverbComponent, "ChiStateMachineComponent는 ReverbComponent가 필요합니다.");

		_context.stateMachine = this;
		_context.beatSystem = &HiFiRushStatics::GetBeatSystem();
		_context.animationSettings = &HiFiRushStatics::GetChiAnimationSettings();
		_context.rhythmJudge = &HiFiRushStatics::GetRhythmJudge();
		_context.moveComponent = _moveComponent;
		_context.targetingComponent = _targetingComponent;
		_context.reverbComponent = _reverbComponent;
		_context.rigidbodyComponent = rigidbodyComponent;
		_context.animatorComponent = _animatorComponent;

		RegisterAnimationClips();
		RegisterStates();

		_healthComponent->OnDamaged.Subscribe(_damagedConnection,
			[this](const HitEvent& event)
			{
				OnDamaged(event);
			});
		if (_weaponHitBox)
		{
			_weaponHitBox->OnHit.Subscribe(_weaponHitConnection,
				[this](const HitEvent& event)
				{
					_reverbComponent->HandleAttackHit(event);
				});
		}

		navMeshControllerComponent->OnGroundContact.Subscribe(_navigationGroundContactConnection,
			[this](const NavigationGroundContactEvent&)
			{
				OnGroundContact();
			});

		_moveComponent->OnMovementBaseContact.Subscribe(_movementBaseContactConnection,
			[this](const MovementBaseContactEvent&)
			{
				OnGroundContact();
			});

		navMeshControllerComponent->OnGroundLost.Subscribe(_groundLostConnection,
			[this](const NavigationGroundLostEvent& event)
			{
				OnGroundLost(event);
			});

		ChangeState(ChiStateId::Idle);
	}

	void ChiStateMachineComponent::OnTick(float deltaTime)
	{
		_context.weakAttackInput.reset();
		_context.strongAttackInput.reset();
		_context.jumpInput.reset();
		_context.dashInput.reset();
		_context.beatHitInput.reset();

		if (_inputEnabled)
		{
			const Input& input = APPLICATION.GetInput();
			if (input.IsMouseDown(MouseButton::Left))
				_context.weakAttackInput = JudgeRhythmInput(RhythmInputType::WeakAttack);

			if (input.IsMouseDown(MouseButton::Right))
				_context.strongAttackInput = JudgeRhythmInput(RhythmInputType::StrongAttack);

			if (input.IsKeyDown(KeyCode::Space))
				_context.jumpInput = JudgeRhythmInput(RhythmInputType::Jump);

			if (input.IsKeyDown(KeyCode::LeftShift))
				_context.dashInput = JudgeRhythmInput(RhythmInputType::Dash);
		}

		UpdateBeatHit();

		ChiState* currentState = FindState(_currentStateId);
		if (currentState == nullptr)
			return;

		currentState->Tick(_context, deltaTime);
	}

	RhythmJudgeResult ChiStateMachineComponent::JudgeRhythmInput(RhythmInputType inputType)
	{
		return _context.rhythmJudge->Judge(*_context.beatSystem, inputType);
	}

	bool ChiStateMachineComponent::BeginBeatHit(BeatHitInputType inputType, uint32 targetBeatOffset)
	{
		GM_ASSERT_RETURN_VAL(targetBeatOffset > 0, false, "Beat Hit 목표 Beat Offset은 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(_context.beatSystem->HasPlaybackTime(), false, "Beat 재생 중에만 Beat Hit을 시작할 수 있습니다.");

		CancelBeatHit();

		ActiveBeatHit beatHit{};
		beatHit.inputType = inputType;
		beatHit.ownerStateId = _currentStateId;
		beatHit.targetBeatIndex = static_cast<int64>(std::round(_context.beatSystem->GetCurrentBeat())) + targetBeatOffset;
		beatHit.approachDurationBeats = static_cast<float>(targetBeatOffset);
		_activeBeatHit = beatHit;

		BeatHitStartedEvent event{};
		event.targetBeat = static_cast<float>(beatHit.targetBeatIndex);
		event.approachDurationBeats = beatHit.approachDurationBeats;
		OnBeatHitStarted.Publish(event);
		return true;
	}

	void ChiStateMachineComponent::SetInputEnabled(bool enabled)
	{
		_inputEnabled = enabled;
		if (_inputEnabled == false)
			CancelBeatHit();
	}

	void ChiStateMachineComponent::CompleteDeathAnimation()
	{
		PlayerDeathAnimationCompletedEvent event{};
		OnDeathAnimationCompleted.Publish(event);
	}

	void ChiStateMachineComponent::UpdateBeatHit()
	{
		if (_activeBeatHit.has_value() == false)
			return;

		const ActiveBeatHit& beatHit = _activeBeatHit.value();
		const RhythmJudgeResult* acceptedInput = nullptr;
		if (beatHit.inputType != BeatHitInputType::StrongAttack && IsAcceptedBeatHitInput(_context.weakAttackInput, beatHit.targetBeatIndex))
			acceptedInput = &_context.weakAttackInput.value();
		else if (beatHit.inputType != BeatHitInputType::WeakAttack && IsAcceptedBeatHitInput(_context.strongAttackInput, beatHit.targetBeatIndex))
			acceptedInput = &_context.strongAttackInput.value();

		if (acceptedInput)
		{
			FinishBeatHit(BeatHitResult::Success, acceptedInput);
			return;
		}

		if (_context.rhythmJudge->HasPassedInputDeadline(*_context.beatSystem, static_cast<float>(beatHit.targetBeatIndex)))
			FinishBeatHit(BeatHitResult::Miss);
	}

	bool ChiStateMachineComponent::IsAcceptedBeatHitInput(const std::optional<RhythmJudgeResult>& input, int64 targetBeatIndex) const
	{
		return input.has_value() && input->judgeGrade != RhythmJudgeGrade::OffBeat && input->judgedBeatIndex == targetBeatIndex;
	}

	void ChiStateMachineComponent::FinishBeatHit(BeatHitResult result, const RhythmJudgeResult* rhythmInput)
	{
		if (_activeBeatHit.has_value() == false)
			return;

		const float targetBeat = static_cast<float>(_activeBeatHit->targetBeatIndex);
		_activeBeatHit.reset();
		if (result == BeatHitResult::Success && rhythmInput)
			_context.beatHitInput = *rhythmInput;

		BeatHitResultEvent event{};
		event.result = result;
		event.targetBeat = targetBeat;
		OnBeatHitResult.Publish(event);
	}

	void ChiStateMachineComponent::CancelBeatHit()
	{
		FinishBeatHit(BeatHitResult::Cancelled);
	}

	void ChiStateMachineComponent::OnGroundContact()
	{
		ChiState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->OnGroundContact(_context);
	}

	void ChiStateMachineComponent::OnGroundLost(const NavigationGroundLostEvent& event)
	{
		ChiState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->OnGroundLost(_context, event);
	}

	void ChiStateMachineComponent::OnDamaged(const HitEvent& event)
	{
		if (event.damageResult.state != DamageState::Applied)
			return;

		_moveComponent->FaceDirectionImmediate(-event.GetWorldKnockbackDirection());

		if (event.damageResult.isDead)
		{
			ChangeState(ChiStateId::DamageDead);
			return;
		}

		if (event.damage.hitReactionType == HitReactionType::Airborne)
		{
			ChangeStateToAirborne(event.damage.worldImpulse);
			return;
		}

		if (event.damage.worldImpulse.LengthSquared() > 0.f)
			_context.rigidbodyComponent->AddImpulse(event.damage.worldImpulse);

		switch (event.damage.hitReactionType)
		{
		case HitReactionType::WeakKnockback:
			ChangeState(ChiStateId::DamageWeakKnockback);
			break;

		case HitReactionType::StrongKnockback:
			ChangeState(ChiStateId::DamageStrongKnockback);
			break;

		default:
			break;
		}
	}

	void ChiStateMachineComponent::ChangeStateToAirborne(const Vector3& impulse)
	{
		ResetTransitionOptions();
		_context.airborneImpulse = impulse;
		ChangeStateInternal(ChiStateId::JumpUp);
	}

	void ChiStateMachineComponent::ChangeState(ChiStateId nextStateId)
	{
		ResetTransitionOptions();
		ChangeStateInternal(nextStateId);
	}

	void ChiStateMachineComponent::ChangeState(ChiStateId nextStateId, float blendDuration)
	{
		ResetTransitionOptions();
		_context.blendDuration = blendDuration;
		ChangeStateInternal(nextStateId);
	}

	void ChiStateMachineComponent::ChangeState(ChiStateId nextStateId, const RhythmJudgeResult& rhythmInput)
	{
		ResetTransitionOptions();
		_context.transitionRhythmInput = rhythmInput;
		if (ChangeStateInternal(nextStateId))
			OnRhythmActionStarted.Publish(rhythmInput);
	}

	void ChiStateMachineComponent::ChangeState(ChiStateId nextStateId, float blendDuration, const RhythmJudgeResult& rhythmInput)
	{
		ResetTransitionOptions();
		_context.blendDuration = blendDuration;
		_context.transitionRhythmInput = rhythmInput;
		if (ChangeStateInternal(nextStateId))
			OnRhythmActionStarted.Publish(rhythmInput);
	}

	bool ChiStateMachineComponent::ChangeStateInternal(ChiStateId nextStateId)
	{
		if (_currentStateId == nextStateId)
		{
			ResetTransitionOptions();
			return false;
		}

		ChiState* next = FindState(nextStateId);
		if (next == nullptr)
		{
			ResetTransitionOptions();
			GM_ASSERT(false, "등록되지 않은 ChiState로 전환할 수 없습니다.");
			return false;
		}

		if (_activeBeatHit && _activeBeatHit->ownerStateId == _currentStateId)
			CancelBeatHit();

		ChiState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Exit(_context);

		_currentStateId = nextStateId;
		next->Enter(_context);
		ResetTransitionOptions();
		return true;
	}

	void ChiStateMachineComponent::ResetTransitionOptions()
	{
		_context.blendDuration.reset();
		_context.airborneImpulse.reset();
		_context.transitionRhythmInput.reset();
	}

	void ChiStateMachineComponent::RegisterAnimationClips()
	{
		for (uint32 animationClipIndex = 0; animationClipIndex < ChiAnimationClipIdCount; ++animationClipIndex)
		{
			const ChiAnimationClipId animationClipId = static_cast<ChiAnimationClipId>(animationClipIndex);
			const std::wstring animationClipName = GetChiAnimationClipName(animationClipId);

			if (_animatorComponent->HasClip(animationClipName))
				continue;

			_animatorComponent->AddClip(animationClipName, GetChiAnimationClipKey(animationClipId));
		}
	}

	void ChiStateMachineComponent::RegisterStates()
	{
		_states.emplace(ChiStateId::Idle, std::make_unique<ChiIdleState>());
		_states.emplace(ChiStateId::Run, std::make_unique<ChiRunState>());

		_states.emplace(ChiStateId::AttackWeak0, std::make_unique<ChiWeak0AttackState>());
		_states.emplace(ChiStateId::AttackWeak1, std::make_unique<ChiWeak1AttackState>());
		_states.emplace(ChiStateId::AttackWeak2, std::make_unique<ChiWeak2AttackState>());
		_states.emplace(ChiStateId::AttackWeak3, std::make_unique<ChiWeak3AttackState>());
		_states.emplace(ChiStateId::AttackWeakDash, std::make_unique<ChiWeakDashAttackState>());
		_states.emplace(ChiStateId::AttackStrongDash, std::make_unique<ChiStrongDashAttackState>());
		_states.emplace(ChiStateId::AttackStrong0_0, std::make_unique<ChiStrong0_0AttackState>());
		_states.emplace(ChiStateId::AttackStrong0_1, std::make_unique<ChiStrong0_1AttackState>());
		_states.emplace(ChiStateId::AttackStrong1, std::make_unique<ChiStrong1AttackState>());
		_states.emplace(ChiStateId::AttackStrong2, std::make_unique<ChiStrong2AttackState>());
		_states.emplace(ChiStateId::AttackStrongToWeak1, std::make_unique<ChiStrongToWeak1AttackState>());
		_states.emplace(ChiStateId::AttackStrongToWeak2, std::make_unique<ChiStrongToWeak2AttackState>(_weaponHitBox));
		_states.emplace(ChiStateId::AttackStrongToWeakBeatHit, std::make_unique<ChiStrongToWeakBeatHitAttackState>());
		_states.emplace(ChiStateId::AttackWeakToStrong1, std::make_unique<ChiWeakToStrong1AttackState>());
		_states.emplace(ChiStateId::AttackWeakToStrong2, std::make_unique<ChiWeakToStrong2AttackState>(_weaponHitBox));
		_states.emplace(ChiStateId::AttackDelayedWeak1, std::make_unique<ChiDelayedWeak1AttackState>());
		_states.emplace(ChiStateId::AttackDelayedWeak2, std::make_unique<ChiDelayedWeak2AttackState>());
		_states.emplace(ChiStateId::AttackStump0, std::make_unique<ChiStump0AttackState>());
		_states.emplace(ChiStateId::AttackStump1, std::make_unique<ChiStump1AttackState>());
		_states.emplace(ChiStateId::AttackStump2, std::make_unique<ChiStump2AttackState>());
		_states.emplace(ChiStateId::AttackSky0, std::make_unique<ChiSky0AttackState>());
		_states.emplace(ChiStateId::AttackSky1, std::make_unique<ChiSky1AttackState>());
		_states.emplace(ChiStateId::AttackSky2, std::make_unique<ChiSky2AttackState>());
		_states.emplace(ChiStateId::AttackSky3, std::make_unique<ChiSky3AttackState>());

		_states.emplace(ChiStateId::DashFront, std::make_unique<ChiDashFrontState>());
		_states.emplace(ChiStateId::DashBack, std::make_unique<ChiDashBackState>());
		_states.emplace(ChiStateId::DashLeft, std::make_unique<ChiDashLeftState>());
		_states.emplace(ChiStateId::DashRight, std::make_unique<ChiDashRightState>());
		_states.emplace(ChiStateId::DashDouble, std::make_unique<ChiDashDoubleState>());
		_states.emplace(ChiStateId::DashTriple, std::make_unique<ChiDashTripleState>());
		_states.emplace(ChiStateId::DashSky, std::make_unique<ChiDashSkyState>());
		_states.emplace(ChiStateId::DashSkyFall, std::make_unique<ChiDashSkyFallState>());

		_states.emplace(ChiStateId::JumpUp, std::make_unique<ChiJumpUpState>());
		_states.emplace(ChiStateId::JumpDown, std::make_unique<ChiJumpDownState>());
		_states.emplace(ChiStateId::JumpLanding, std::make_unique<ChiJumpLandingState>());
		_states.emplace(ChiStateId::JumpDoubleUp, std::make_unique<ChiJumpDoubleUpState>());
		_states.emplace(ChiStateId::JumpDoubleDown, std::make_unique<ChiJumpDoubleDownState>());

		_states.emplace(ChiStateId::DamageStrongKnockback, std::make_unique<ChiStrongKnockbackDamageState>());
		_states.emplace(ChiStateId::DamageWeakKnockback, std::make_unique<ChiWeakKnockbackDamageState>());
		_states.emplace(ChiStateId::DamageDead, std::make_unique<ChiDeadDamageState>());

		_states.emplace(ChiStateId::HibikiReady, std::make_unique<ChiHibikiReadyState>());
		_states.emplace(ChiStateId::HibikiAttack, std::make_unique<ChiHibikiAttackState>());
	}

	ChiState* ChiStateMachineComponent::FindState(ChiStateId stateId) const
	{
		auto iter = _states.find(stateId);
		if (iter == _states.end())
			return nullptr;

		return iter->second.get();
	}
}
