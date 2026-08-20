#include "QamilNormalAttackState.h"

#include "AnimationTypes.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushCollisionLayers.h"
#include "MathUtil.h"
#include "QamilStateMachineComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMeshComponent.h"
#include "SocketComponent.h"
#include "SphereCollider3DComponent.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <limits>

namespace gm
{
	namespace
	{
		constexpr uint32 QamilNormalAttackWaitCycleCount = 3;
		constexpr uint32 QamilHandCollisionDelayFrames = 2;
		constexpr int32 QamilNormalAttackDamage = 30;
		constexpr float QamilNormalAttackHitBoxLifetime = 0.1f;
		constexpr float QamilNormalAttackHitBoxRadius = 2.25f;
		constexpr float QamilNormalAttackAimMinZ = -7.f;
		constexpr float QamilNormalAttackAimMaxZ = 3.5f;
		constexpr wchar_t QamilLeftHandColliderId[] = L"Qamil.LeftHand";
		constexpr wchar_t QamilRightHandColliderId[] = L"Qamil.RightHand";
	}

	void QamilNormalAttackState::Enter(QamilStateContext& context)
	{
		_waitCycleCount = 0;
		_isAimLocked = false;
		_targetAimOffset = {};
		_restoreStartAimOffset = {};
		_handCollisionDelayFrames = 0;
		_attackAim.Initialize(context);
		SelectAttack(context);
		BeginAttack(context);
	}

	void QamilNormalAttackState::Tick(QamilStateContext& context, float deltaTime)
	{
		UpdateHandCollisionBlocking();
		UpdateAimCorrection(context);
		if (IsAnimationCompleted(context) == false)
			return;

		switch (_phase)
		{
		case Phase::Attack:
			BeginWait(context);
			break;
		case Phase::Wait:
			++_waitCycleCount;
			if (_waitCycleCount >= QamilNormalAttackWaitCycleCount)
				BeginRestore(context);
			else
				PlayBeatSyncedAnimation(context, _waitAnimationId, false, 0.f);
			break;
		case Phase::Restore:
			_attackAim.Reset(context);
			context.stateMachine->ChangeState(QamilStateId::Idle);
			break;
		}
	}

	void QamilNormalAttackState::Exit(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		_handCollisionDelayFrames = 0;
		SetHandCollisionBlocking(false);
		_attackAim.Reset(context);
		if (context.animatorComponent)
			context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	void QamilNormalAttackState::SelectAttack(QamilStateContext& context)
	{
		struct AttackSetting
		{
			Vector3 referencePosition;
			QamilAnimationId attackAnimationId;
			QamilAnimationId waitAnimationId;
			QamilAnimationId restoreAnimationId;
			const wchar_t* handColliderId;
		};

		constexpr std::array<AttackSetting, 6> AttackSettings =
		{{
			{ { -9.53f, 0.f, -7.74f }, QamilAnimationId::NormalLeftOuterNear, QamilAnimationId::NormalLeftOuterNearWait, QamilAnimationId::NormalLeftOuterNearRestore, QamilLeftHandColliderId },
			{ { -12.78f, 0.f, 0.16f }, QamilAnimationId::NormalLeftOuterFar, QamilAnimationId::NormalLeftOuterFarWait, QamilAnimationId::NormalLeftOuterFarRestore, QamilLeftHandColliderId },
			{ { 0.09f, 0.f, -11.77f }, QamilAnimationId::NormalLeftNear, QamilAnimationId::NormalLeftNearWait, QamilAnimationId::NormalLeftNearRestore, QamilLeftHandColliderId },
			{ { 0.36f, 0.f, 0.42f }, QamilAnimationId::NormalLeftFar, QamilAnimationId::NormalLeftFarWait, QamilAnimationId::NormalLeftFarRestore, QamilLeftHandColliderId },
			{ { 9.63f, 0.f, -7.74f }, QamilAnimationId::NormalRightOuterNear, QamilAnimationId::NormalRightOuterNearWait, QamilAnimationId::NormalRightOuterNearRestore, QamilRightHandColliderId },
			{ { 12.68f, 0.f, 0.15f }, QamilAnimationId::NormalRightOuterFar, QamilAnimationId::NormalRightOuterFarWait, QamilAnimationId::NormalRightOuterFarRestore, QamilRightHandColliderId },
		}};

		Vector3 targetPosition{};
		GM_ASSERT_RETURN(_attackAim.ResolveTargetCenter(context, targetPosition), "Qamil Normal Attack 대상을 찾을 수 없습니다.");
		const Matrix world = context.transformComponent->GetWorldMatrix();
		const AttackSetting* selectedSetting = nullptr;
		float nearestDistanceSquared = (std::numeric_limits<float>::max)();
		for (const AttackSetting& setting : AttackSettings)
		{
			const Vector3 referencePosition = Vector3::Transform(setting.referencePosition, world);
			const Vector3 offset = Math::ProjectOnXZPlane(referencePosition - targetPosition);
			const float distanceSquared = offset.LengthSquared();
			if (distanceSquared >= nearestDistanceSquared)
				continue;

			selectedSetting = &setting;
			nearestDistanceSquared = distanceSquared;
		}

		GM_ASSERT_RETURN(selectedSetting, "Qamil Normal Attack 위치를 선택하지 못했습니다.");
		_attackAnimationId = selectedSetting->attackAnimationId;
		_waitAnimationId = selectedSetting->waitAnimationId;
		_restoreAnimationId = selectedSetting->restoreAnimationId;
		_attackReferencePosition = selectedSetting->referencePosition;
		_handCollider = nullptr;
		for (Collider3DComponent* collider : context.stateMachine->GetOwner().GetColliders3D())
		{
			if (collider->GetColliderId() == selectedSetting->handColliderId)
			{
				_handCollider = collider;
				break;
			}
		}
		GM_ASSERT(_handCollider, "Qamil Normal Attack Hand Collider를 찾을 수 없습니다. collider=%ls", selectedSetting->handColliderId);
	}

	void QamilNormalAttackState::BeginAttack(QamilStateContext& context)
	{
		_phase = Phase::Attack;
		_notifyConnection.Disconnect();
		GM_ASSERT_RETURN(PlayAnimation(context, _attackAnimationId, false), "Qamil Normal Attack Animation 재생에 실패했습니다.");
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip, "Qamil Normal Attack Animation Clip이 없습니다.");
		const AnimationNotifyEvent* hitStartNotify = clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart);
		GM_ASSERT_RETURN(hitStartNotify, "Qamil Normal Attack Animation에 HitStart Notify가 없습니다.");
		_hitTime = hitStartNotify->time;
		const float animationSecondsPerBeat = clip->GetTicksPerSecond() > 0.f ? QamilAnimationTicksPerBeat / clip->GetTicksPerSecond() : 0.f;
		_aimStartTime = std::max(0.f, _hitTime - animationSecondsPerBeat);
		context.animatorComponent->GetNotifyEvent().Subscribe(_notifyConnection, [this, &context](const AnimationNotifyEvent& event) { HandleAnimationNotify(context, event); });
		if (context.beatSystem && context.beatSystem->HasPlaybackTime())
			context.animatorComponent->SetPlayRate(BeatMath::CalcAnimationPlayRate(context.beatSystem->GetCurrentBeat(), context.beatSystem->GetSecondsPerBeat(), hitStartNotify->time));
		else
			context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	void QamilNormalAttackState::BeginWait(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		_phase = Phase::Wait;
		PlayBeatSyncedAnimation(context, _waitAnimationId, false);
	}

	void QamilNormalAttackState::BeginRestore(QamilStateContext& context)
	{
		_phase = Phase::Restore;
		_handCollisionDelayFrames = 0;
		SetHandCollisionBlocking(false);
		_restoreStartAimOffset = _attackAim.GetCurrentOffset();
		PlayBeatSyncedAnimation(context, _restoreAnimationId, false);
	}

	void QamilNormalAttackState::UpdateAimCorrection(QamilStateContext& context)
	{
		if (_phase == Phase::Wait)
			return;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr)
			return;

		const float playTime = context.animatorComponent->GetPlayTime();
		if (_phase == Phase::Attack)
		{
			if (_isAimLocked == false && playTime >= _aimStartTime)
				LockAimCorrection(context);
			if (_isAimLocked == false)
				return;

			const float duration = _hitTime - _aimStartTime;
			const float ratio = duration > 0.f ? std::clamp((playTime - _aimStartTime) / duration, 0.f, 1.f) : 1.f;
			const float smoothRatio = ratio * ratio * (3.f - 2.f * ratio);
			_attackAim.Apply(context, Vector3::Lerp(Vector3{}, _targetAimOffset, smoothRatio));
			return;
		}

		const float ratio = clip->GetLength() > 0.f ? std::clamp(playTime / clip->GetLength(), 0.f, 1.f) : 1.f;
		const float smoothRatio = ratio * ratio * (3.f - 2.f * ratio);
		_attackAim.Apply(context, Vector3::Lerp(_restoreStartAimOffset, Vector3{}, smoothRatio));
	}

	void QamilNormalAttackState::LockAimCorrection(QamilStateContext& context)
	{
		GM_ASSERT_RETURN(_attackAim.CalculateTargetOffset(context, _attackReferencePosition, QamilNormalAttackAimMinZ, QamilNormalAttackAimMaxZ, _targetAimOffset), "Qamil Normal Attack 조준 대상을 찾을 수 없습니다.");
		_isAimLocked = true;
	}

	void QamilNormalAttackState::UpdateHandCollisionBlocking()
	{
		if (_handCollisionDelayFrames == 0)
			return;

		--_handCollisionDelayFrames;
		if (_handCollisionDelayFrames == 0)
			SetHandCollisionBlocking(true);
	}

	void QamilNormalAttackState::SetHandCollisionBlocking(bool isBlocking)
	{
		if (_handCollider)
			_handCollider->SetTrigger(isBlocking == false);
	}

	void QamilNormalAttackState::HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name != HiFiRushAnimationNotifyNames::HitStart)
			return;
		if (_isAimLocked == false)
			LockAimCorrection(context);
		_attackAim.Apply(context, _targetAimOffset);
		GM_ASSERT_RETURN(SpawnHitBox(context), "Qamil Normal Attack HitBox 생성에 실패했습니다.");
		_handCollisionDelayFrames = QamilHandCollisionDelayFrames;
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	bool QamilNormalAttackState::SpawnHitBox(QamilStateContext& context)
	{
		if (context.stateMachine == nullptr || _handCollider == nullptr || _handCollider->GetShapeType() != ColliderShape3DType::Sphere)
			return false;

		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		if (scene == nullptr)
			return false;
		const SphereCollider3DComponent& handCollider = static_cast<const SphereCollider3DComponent&>(*_handCollider);
		const BoundingSphere& handShape = handCollider.GetWorldShape();

		TemporarySphereHitBoxDesc desc{};
		desc.world = Matrix::CreateTranslation(handShape.Center);
		desc.colliderId = L"Attack";
		desc.radius = QamilNormalAttackHitBoxRadius;
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = QamilNormalAttackDamage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.lifetime = QamilNormalAttackHitBoxLifetime;
		return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
	}
}
