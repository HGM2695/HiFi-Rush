#include "QamilSpecialAttackState.h"

#include "AudioStatics.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushAudio.h"
#include "MathUtil.h"
#include "QamilEffectComponent.h"
#include "QamilStateMachineComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMeshComponent.h"
#include "SphereCollider3DComponent.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace gm
{
	namespace
	{
		constexpr wchar_t QamilLeftHandColliderId[] = L"Qamil.LeftHand";
		constexpr wchar_t QamilRightHandColliderId[] = L"Qamil.RightHand";
		constexpr int32 QamilSpecialAttackDamage = 30;
		constexpr int32 QamilChainAttackDamage = 20;
		constexpr float QamilSpecialAttackHitBoxLifetime = 0.1f;
		constexpr float QamilChainAttackHitBoxRadius = 2.6f;
		constexpr float QamilLaserLength = 80.f;
		constexpr Vector3 QamilLaserHitBoxSize{ 4.f, 3.f, QamilLaserLength };
		constexpr float QamilLaserReboundHandRadius = 3.3f;
		constexpr uint32 QamilLaserReboundCycleCount = 4;
		constexpr float QamilLaserReboundStartBeat = 12.f;
		constexpr float QamilLaserCollisionReleaseBeat = 4.f;
		constexpr float QamilLaserRestoreEffectIntervalBeats = 1.5f;
		constexpr std::array QamilLaserAttackBeats{ 4.f, 4.f + 7.f / 15.f, 5.f, 5.f + 7.f / 15.f, 8.f, 8.f + 7.f / 15.f, 9.f, 9.f + 7.f / 15.f };
		constexpr uint32 QamilChainAttackCount = 3;
		constexpr uint32 QamilChainWaitCycleCount = 6;
		constexpr uint32 QamilHandCollisionDelayFrames = 2;
		constexpr float QamilChainFinisherHitBeat = 2.f;
		constexpr float QamilChainHandCollisionReleaseBeat = 3.f / 15.f;
		constexpr float QamilChainFinisherCollisionReleaseBeat = 2.f;
		constexpr float QamilChainAimMinZ = -7.f;
		constexpr float QamilChainAimMaxZ = 3.5f;

		struct QamilChainAttackSetting
		{
			Vector3 referencePosition;
			const wchar_t* handColliderId;
			QamilAnimationId attackAnimationId;
			QamilAnimationId returnToLeftAnimationId;
			QamilAnimationId returnToRightAnimationId;
			QamilAnimationId finisherAnimationId;
			QamilAnimationId finisherWaitAnimationId;
			QamilAnimationId finisherToMooyahoAnimationId;
			bool isLeft;
		};

		constexpr std::array QamilChainAttackSettings
		{
			QamilChainAttackSetting{ { -9.17f, 0.f, -7.57f }, QamilLeftHandColliderId, QamilAnimationId::ChainLeftReadyToLeftOuterNear, QamilAnimationId::ChainLeftOuterNearToLeftReady, QamilAnimationId::ChainLeftOuterNearToRightReady, QamilAnimationId::ChainLeftReadyToLeftOuterNearFinisher, QamilAnimationId::ChainLeftOuterNearFinisherToWait, QamilAnimationId::ChainLeftOuterNearFinisherToMooyaho, true },
			QamilChainAttackSetting{ { -12.21f, 0.f, -0.19f }, QamilLeftHandColliderId, QamilAnimationId::ChainLeftReadyToLeftOuterFar, QamilAnimationId::ChainLeftOuterFarToLeftReady, QamilAnimationId::ChainLeftOuterFarToRightReady, QamilAnimationId::ChainLeftReadyToLeftOuterFarFinisher, QamilAnimationId::ChainLeftOuterFarFinisherToWait, QamilAnimationId::ChainLeftOuterFarFinisherToMooyaho, true },
			QamilChainAttackSetting{ { 0.02f, 0.f, -12.23f }, QamilLeftHandColliderId, QamilAnimationId::ChainLeftReadyToLeftNear, QamilAnimationId::ChainLeftNearToLeftReady, QamilAnimationId::ChainLeftNearToRightReady, QamilAnimationId::ChainLeftReadyToLeftNearFinisher, QamilAnimationId::ChainLeftNearFinisherToWait, QamilAnimationId::ChainLeftNearFinisherToMooyaho, true },
			QamilChainAttackSetting{ { 0.04f, 0.f, 0.23f }, QamilLeftHandColliderId, QamilAnimationId::ChainLeftReadyToLeftFar, QamilAnimationId::ChainLeftFarToLeftReady, QamilAnimationId::ChainLeftFarToRightReady, QamilAnimationId::ChainLeftReadyToLeftFarFinisher, QamilAnimationId::ChainLeftFarFinisherToWait, QamilAnimationId::ChainLeftFarFinisherToMooyaho, true },
			QamilChainAttackSetting{ { 9.26f, 0.f, -7.27f }, QamilRightHandColliderId, QamilAnimationId::ChainRightReadyToRightOuterNear, QamilAnimationId::ChainRightOuterNearToLeftReady, QamilAnimationId::ChainRightOuterNearToRightReady, QamilAnimationId::ChainRightReadyToRightOuterNearFinisher, QamilAnimationId::ChainRightOuterNearFinisherToWait, QamilAnimationId::ChainRightOuterNearFinisherToMooyaho, false },
			QamilChainAttackSetting{ { 12.4f, 0.f, 0.1f }, QamilRightHandColliderId, QamilAnimationId::ChainRightReadyToRightOuterFar, QamilAnimationId::ChainRightOuterFarToLeftReady, QamilAnimationId::ChainRightOuterFarToRightReady, QamilAnimationId::ChainRightReadyToRightOuterFarFinisher, QamilAnimationId::ChainRightOuterFarFinisherToWait, QamilAnimationId::ChainRightOuterFarFinisherToMooyaho, false },
		};

		Collider3DComponent* FindCollider(GameObject& owner, const wchar_t* colliderId)
		{
			for (Collider3DComponent* collider : owner.GetColliders3D())
			{
				if (collider->GetColliderId() == colliderId)
					return collider;
			}
			return nullptr;
		}

		Vector3 GetAxisAlignedDirection(const Vector3& direction)
		{
			if (std::abs(direction.x) >= std::abs(direction.z))
				return Vector3{ direction.x >= 0.f ? 1.f : -1.f, 0.f, 0.f };
			return Vector3{ 0.f, 0.f, direction.z >= 0.f ? 1.f : -1.f };
		}
	}

	/// QamilLaserState //////////////////////////////////////////////////////////////////////////////////////////////////
	void QamilLaserState::Enter(QamilStateContext& context)
	{
		_phase = Phase::Attack;
		_nextLaserIndex = 0;
		_reboundCycleCount = 0;
		_restoreEffectElapsedBeats = 0.f;
		_previousBeat = context.beatSystem->HasPlaybackTime() ? std::optional<float>{ context.beatSystem->GetCurrentBeat() } : std::nullopt;
		_isReboundCollisionEnabled = false;
		GM_ASSERT_RETURN(ResolveHandColliders(context), "Qamil Laser Hand Collider를 찾을 수 없습니다.");
		GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::Laser, false), "Qamil Laser Animation 재생에 실패했습니다.");
	}

	void QamilLaserState::Tick(QamilStateContext& context, float)
	{
		UpdateRestoreEffect(context);
		if (_phase == Phase::Attack)
		{
			const float animationBeat = GetAnimationBeat(context);
			while (_nextLaserIndex < QamilLaserAttackBeats.size() && animationBeat >= QamilLaserAttackBeats[_nextLaserIndex])
			{
				GM_ASSERT(SpawnLaserAttack(context, _nextLaserIndex), "Qamil Laser 공격 생성에 실패했습니다. index=%u", _nextLaserIndex);
				if (_nextLaserIndex == 0 || _nextLaserIndex == QamilLaserAttackBeats.size() / 2)
					PlaySound2D(HiFiRushSound::QamilLaser);
				++_nextLaserIndex;
			}
			if (_isReboundCollisionEnabled == false && animationBeat >= QamilLaserReboundStartBeat)
				SetReboundCollision(true);
		}
		else if (_phase == Phase::Restore && _isReboundCollisionEnabled && GetAnimationBeat(context) >= QamilLaserCollisionReleaseBeat)
		{
			SetReboundCollision(false);
		}

		if (IsAnimationCompleted(context) == false)
			return;

		switch (_phase)
		{
		case Phase::Attack:
			_phase = Phase::Rebound;
			GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::LaserRebound, false, 0.f), "Qamil Laser Rebound Animation 재생에 실패했습니다.");
			break;
		case Phase::Rebound:
			++_reboundCycleCount;
			if (_reboundCycleCount >= QamilLaserReboundCycleCount)
			{
				_phase = Phase::Restore;
				GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::LaserRestore, false, 0.f), "Qamil Laser Restore Animation 재생에 실패했습니다.");
			}
			else
			{
				GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::LaserRebound, false, 0.f), "Qamil Laser Rebound Animation 반복 재생에 실패했습니다.");
			}
			break;
		case Phase::Restore:
			context.stateMachine->ChangeState(QamilStateId::Idle);
			break;
		}
	}

	void QamilLaserState::Exit(QamilStateContext& context)
	{
		SetReboundCollision(false);
		_previousBeat.reset();
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	bool QamilLaserState::ResolveHandColliders(QamilStateContext& context)
	{
		GameObject& owner = context.stateMachine->GetOwner();
		_leftHandCollider = FindCollider(owner, QamilLeftHandColliderId);
		_rightHandCollider = FindCollider(owner, QamilRightHandColliderId);
		if (_leftHandCollider == nullptr || _rightHandCollider == nullptr || _leftHandCollider->GetShapeType() != ColliderShape3DType::Sphere || _rightHandCollider->GetShapeType() != ColliderShape3DType::Sphere)
			return false;
		_leftHandBaseRadius = static_cast<SphereCollider3DComponent*>(_leftHandCollider)->GetRadius();
		_rightHandBaseRadius = static_cast<SphereCollider3DComponent*>(_rightHandCollider)->GetRadius();
		return true;
	}

	bool QamilLaserState::SpawnLaserAttack(QamilStateContext& context, uint32 laserIndex) const
	{
		if (laserIndex >= QamilLaserAttackBeats.size() || context.stateMachine == nullptr || context.transformComponent == nullptr)
			return false;
		Scene* scene = context.stateMachine->GetOwner().GetScene();
		Collider3DComponent* handCollider = laserIndex < QamilLaserAttackBeats.size() / 2 ? _rightHandCollider : _leftHandCollider;
		if (scene == nullptr || handCollider == nullptr || handCollider->GetShapeType() != ColliderShape3DType::Sphere)
			return false;

		Vector3 center = static_cast<SphereCollider3DComponent*>(handCollider)->GetWorldShape().Center;
		center.y = context.transformComponent->GetPosition().y + QamilLaserHitBoxSize.y * 0.5f;
		const Vector3 platformInwardOffset = context.transformComponent->GetPosition() - GetCurrentPlatformPosition(context);
		if (platformInwardOffset.LengthSquared() <= 0.000001f)
			return false;
		const Vector3 platformInwardDirection = GetAxisAlignedDirection(platformInwardOffset);

		TemporaryBoxHitBoxDesc desc{};
		desc.world = Math::CreateTransformMatrix(center, Math::CreateRotationByDirection(platformInwardDirection));
		desc.colliderId = L"Qamil.Laser";
		desc.size = QamilLaserHitBoxSize;
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = QamilSpecialAttackDamage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.damageInfo.worldKnockbackDirection = platformInwardDirection;
		desc.lifetime = QamilSpecialAttackHitBoxLifetime;
		if (scene->SpawnGameObject<TemporaryHitBoxObject>(desc) == nullptr)
			return false;
		QamilEffectComponent* effectComponent = context.stateMachine->GetOwner().GetComponent<QamilEffectComponent>();
		return effectComponent && effectComponent->SpawnLaserStrike(center, platformInwardDirection);
	}

	void QamilLaserState::UpdateRestoreEffect(QamilStateContext& context)
	{
		if (context.beatSystem->HasPlaybackTime() == false)
		{
			_previousBeat.reset();
			return;
		}

		const float currentBeat = context.beatSystem->GetCurrentBeat();
		const float beatDelta = _previousBeat.has_value() ? std::max(0.f, currentBeat - _previousBeat.value()) : 0.f;
		_previousBeat = currentBeat;
		const bool shouldSpawnRestoreEffect = (_phase == Phase::Attack && GetAnimationBeat(context) >= QamilLaserReboundStartBeat) || _phase == Phase::Rebound;
		if (shouldSpawnRestoreEffect == false)
			return;

		_restoreEffectElapsedBeats += beatDelta;
		while (_restoreEffectElapsedBeats >= QamilLaserRestoreEffectIntervalBeats)
		{
			_restoreEffectElapsedBeats -= QamilLaserRestoreEffectIntervalBeats;
			const Vector3 leftHandCenter = static_cast<SphereCollider3DComponent*>(_leftHandCollider)->GetWorldShape().Center;
			const Vector3 rightHandCenter = static_cast<SphereCollider3DComponent*>(_rightHandCollider)->GetWorldShape().Center;
			QamilEffectComponent* effectComponent = context.stateMachine->GetOwner().GetComponent<QamilEffectComponent>();
			GM_ASSERT(effectComponent && effectComponent->SpawnLaserRestoreSmoke(leftHandCenter, rightHandCenter), "Qamil Laser Restore Smoke 생성에 실패했습니다.");
		}
	}

	void QamilLaserState::SetReboundCollision(bool isEnabled)
	{
		if (_leftHandCollider == nullptr || _rightHandCollider == nullptr)
			return;
		SphereCollider3DComponent* leftHand = static_cast<SphereCollider3DComponent*>(_leftHandCollider);
		SphereCollider3DComponent* rightHand = static_cast<SphereCollider3DComponent*>(_rightHandCollider);
		leftHand->SetRadius(isEnabled ? QamilLaserReboundHandRadius : _leftHandBaseRadius);
		rightHand->SetRadius(isEnabled ? QamilLaserReboundHandRadius : _rightHandBaseRadius);
		leftHand->SetTrigger(isEnabled == false);
		rightHand->SetTrigger(isEnabled == false);
		_isReboundCollisionEnabled = isEnabled;
	}

	/// QamilChainState /////////////////////////////////////////////////////////////////////////////////////////////////
	void QamilChainState::Enter(QamilStateContext& context)
	{
		_phase = Phase::Opening;
		_attackCount = 0;
		_waitCycleCount = 0;
		_handCollisionDelayFrames = 0;
		_hasSpawnedFinisherHitBox = false;
		_blockingHandCollider = nullptr;
		_transitionStartAimOffset = {};
		_targetAimOffset = {};
		_attackAim.Initialize(context);
		GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::Wooth, false), "Qamil Chain Opening Animation 재생에 실패했습니다.");
		PlaySound2D(HiFiRushSound::QamilChain);
	}

	void QamilChainState::Tick(QamilStateContext& context, float)
	{
		UpdateAimCorrection(context);
		UpdateHandCollisionBlocking(context);
		if (_phase == Phase::Strike && _attackCount == QamilChainAttackCount && _hasSpawnedFinisherHitBox == false && GetAnimationBeat(context) >= QamilChainFinisherHitBeat)
		{
			GM_ASSERT_RETURN(SpawnHitBox(context), "Qamil Chain Finisher HitBox 생성에 실패했습니다.");
			_hasSpawnedFinisherHitBox = true;
		}
		if (IsAnimationCompleted(context) == false)
			return;

		switch (_phase)
		{
		case Phase::Opening:
			GM_ASSERT_RETURN(SelectNextAttack(context), "Qamil Chain 첫 공격을 선택하지 못했습니다.");
			GM_ASSERT_RETURN(BeginReadyTransition(context), "Qamil Chain Ready 전환에 실패했습니다.");
			break;
		case Phase::ReadyTransition:
		case Phase::ReturnToReady:
			_attackAim.Apply(context, _targetAimOffset);
			GM_ASSERT_RETURN(BeginStrike(context), "Qamil Chain 공격 Animation 재생에 실패했습니다.");
			break;
		case Phase::Strike:
			if (_attackCount < QamilChainAttackCount)
			{
				GM_ASSERT_RETURN(SpawnHitBox(context), "Qamil Chain HitBox 생성에 실패했습니다.");
				GM_ASSERT_RETURN(BeginReturnToReady(context), "Qamil Chain 다음 공격 준비에 실패했습니다.");
			}
			else
				GM_ASSERT_RETURN(BeginFinisherWait(context), "Qamil Chain Finisher 대기 전환에 실패했습니다.");
			break;
		case Phase::FinisherWait:
			++_waitCycleCount;
			if (_waitCycleCount >= QamilChainWaitCycleCount)
				GM_ASSERT_RETURN(BeginMooyaho(context), "Qamil Chain Mooyaho 전환에 실패했습니다.");
			else
				GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilChainAttackSettings[_attackSettingIndex].finisherWaitAnimationId, false, 0.f), "Qamil Chain Finisher 대기 Animation 반복 재생에 실패했습니다.");
			break;
		case Phase::FinisherToMooyaho:
			SetHandCollisionBlocking(false);
			_attackAim.Apply(context, {});
			_phase = Phase::Restore;
			GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::MooyahoToWoong, false, 0.f), "Qamil Chain 복귀 Animation 재생에 실패했습니다.");
			break;
		case Phase::Restore:
			context.stateMachine->ChangeState(QamilStateId::Idle);
			break;
		}
	}

	void QamilChainState::Exit(QamilStateContext& context)
	{
		_handCollisionDelayFrames = 0;
		SetHandCollisionBlocking(false);
		_attackAim.Reset(context);
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	bool QamilChainState::SelectNextAttack(QamilStateContext& context)
	{
		Vector3 targetPosition{};
		if (_attackAim.ResolveTargetCenter(context, targetPosition) == false || context.transformComponent == nullptr)
			return false;
		const Matrix world = context.transformComponent->GetWorldMatrix();
		float nearestDistanceSquared = (std::numeric_limits<float>::max)();
		for (uint32 settingIndex = 0; settingIndex < QamilChainAttackSettings.size(); ++settingIndex)
		{
			const Vector3 referencePosition = Vector3::Transform(QamilChainAttackSettings[settingIndex].referencePosition, world);
			const float distanceSquared = Math::ProjectOnXZPlane(referencePosition - targetPosition).LengthSquared();
			if (distanceSquared >= nearestDistanceSquared)
				continue;
			_attackSettingIndex = settingIndex;
			nearestDistanceSquared = distanceSquared;
		}
		const QamilChainAttackSetting& setting = QamilChainAttackSettings[_attackSettingIndex];
		_transitionStartAimOffset = _attackAim.GetCurrentOffset();
		return _attackAim.CalculateTargetOffset(context, setting.referencePosition, QamilChainAimMinZ, QamilChainAimMaxZ, _targetAimOffset);
	}

	bool QamilChainState::BeginReadyTransition(QamilStateContext& context)
	{
		_phase = Phase::ReadyTransition;
		const QamilChainAttackSetting& setting = QamilChainAttackSettings[_attackSettingIndex];
		return PlayBeatSyncedAnimation(context, setting.isLeft ? QamilAnimationId::ChainWoothToLeftReady : QamilAnimationId::ChainWoothToRightReady, false, 0.f);
	}

	bool QamilChainState::BeginStrike(QamilStateContext& context)
	{
		SetHandCollisionBlocking(false);
		++_attackCount;
		_hasSpawnedFinisherHitBox = false;
		_phase = Phase::Strike;
		const QamilChainAttackSetting& setting = QamilChainAttackSettings[_attackSettingIndex];
		return PlayBeatSyncedAnimation(context, _attackCount == QamilChainAttackCount ? setting.finisherAnimationId : setting.attackAnimationId, false, 0.f);
	}

	bool QamilChainState::BeginReturnToReady(QamilStateContext& context)
	{
		const QamilChainAttackSetting& previousSetting = QamilChainAttackSettings[_attackSettingIndex];
		if (SelectNextAttack(context) == false)
			return false;
		const QamilChainAttackSetting& nextSetting = QamilChainAttackSettings[_attackSettingIndex];
		_phase = Phase::ReturnToReady;
		return PlayBeatSyncedAnimation(context, nextSetting.isLeft ? previousSetting.returnToLeftAnimationId : previousSetting.returnToRightAnimationId, false, 0.f);
	}

	bool QamilChainState::BeginFinisherWait(QamilStateContext& context)
	{
		_phase = Phase::FinisherWait;
		_waitCycleCount = 0;
		return PlayBeatSyncedAnimation(context, QamilChainAttackSettings[_attackSettingIndex].finisherWaitAnimationId, false, 0.f);
	}

	bool QamilChainState::BeginMooyaho(QamilStateContext& context)
	{
		_phase = Phase::FinisherToMooyaho;
		_transitionStartAimOffset = _attackAim.GetCurrentOffset();
		_targetAimOffset = {};
		return PlayBeatSyncedAnimation(context, QamilChainAttackSettings[_attackSettingIndex].finisherToMooyahoAnimationId, false, 0.f);
	}

	bool QamilChainState::SpawnHitBox(QamilStateContext& context)
	{
		if (context.stateMachine == nullptr)
			return false;
		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		const QamilChainAttackSetting& setting = QamilChainAttackSettings[_attackSettingIndex];
		Collider3DComponent* handCollider = FindCollider(owner, setting.handColliderId);
		if (scene == nullptr || handCollider == nullptr || handCollider->GetShapeType() != ColliderShape3DType::Sphere)
			return false;
		const BoundingSphere& handShape = static_cast<SphereCollider3DComponent*>(handCollider)->GetWorldShape();

		TemporarySphereHitBoxDesc desc{};
		desc.world = Matrix::CreateTranslation(handShape.Center);
		desc.colliderId = L"Qamil.Chain";
		desc.radius = QamilChainAttackHitBoxRadius;
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = QamilChainAttackDamage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.lifetime = QamilSpecialAttackHitBoxLifetime;
		if (scene->SpawnGameObject<TemporaryHitBoxObject>(desc) == nullptr)
			return false;
		QamilEffectComponent* effectComponent = owner.GetComponent<QamilEffectComponent>();
		const bool hasSpawnedPunchImpact = effectComponent && effectComponent->SpawnPunchImpact(handShape.Center, handShape.Radius);
		GM_ASSERT(hasSpawnedPunchImpact, "Qamil Chain Punch Impact 생성에 실패했습니다.");
		_blockingHandCollider = handCollider;
		_handCollisionDelayFrames = QamilHandCollisionDelayFrames;
		return true;
	}

	void QamilChainState::UpdateAimCorrection(QamilStateContext& context)
	{
		if (_phase != Phase::ReadyTransition && _phase != Phase::ReturnToReady && _phase != Phase::FinisherToMooyaho)
			return;
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr)
			return;
		const float ratio = clip->GetLength() > 0.f ? std::clamp(context.animatorComponent->GetPlayTime() / clip->GetLength(), 0.f, 1.f) : 1.f;
		const float smoothRatio = ratio * ratio * (3.f - 2.f * ratio);
		_attackAim.Apply(context, Vector3::Lerp(_transitionStartAimOffset, _targetAimOffset, smoothRatio));
	}

	void QamilChainState::UpdateHandCollisionBlocking(QamilStateContext& context)
	{
		if (_handCollisionDelayFrames > 0)
		{
			--_handCollisionDelayFrames;
			if (_handCollisionDelayFrames == 0)
				SetHandCollisionBlocking(true);
		}
		if (_blockingHandCollider == nullptr)
			return;
		if (_phase == Phase::ReturnToReady && GetAnimationBeat(context) >= QamilChainHandCollisionReleaseBeat)
			SetHandCollisionBlocking(false);
		else if (_phase == Phase::FinisherToMooyaho && GetAnimationBeat(context) >= QamilChainFinisherCollisionReleaseBeat)
			SetHandCollisionBlocking(false);
	}

	void QamilChainState::SetHandCollisionBlocking(bool isBlocking)
	{
		if (_blockingHandCollider)
			_blockingHandCollider->SetTrigger(isBlocking == false);
		if (isBlocking == false)
			_blockingHandCollider = nullptr;
	}
}
