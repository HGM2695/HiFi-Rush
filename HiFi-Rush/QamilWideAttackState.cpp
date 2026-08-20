#include "QamilWideAttackState.h"

#include "AnimationTypes.h"
#include "GameObject.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushCollisionLayers.h"
#include "HurtBoxComponent.h"
#include "MathUtil.h"
#include "QamilStateMachineComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"
#include "TriggerSystem.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr int32 QamilWideAttackDamage = 30;
		constexpr float QamilWideAttackHitBoxLifetime = 0.1f;
		constexpr Vector3 QamilStumpHitBoxSize{ 30.f, 4.f, 30.f };
		constexpr float QamilStumpGroundTolerance = 0.1f;
		constexpr float QamilStumpVerticalImpulse = 30.f;
		constexpr float QamilSweepRadius = 26.f;
		constexpr float QamilPlatformRotation = Math::GM_PI * 0.5f;
		constexpr wchar_t QamilStumpShakePositiveXTriggerId[] = L"Qamil.StumpShake.PositiveX";
		constexpr wchar_t QamilStumpShakeNegativeXTriggerId[] = L"Qamil.StumpShake.NegativeX";
		constexpr wchar_t QamilStumpShakePositiveZTriggerId[] = L"Qamil.StumpShake.PositiveZ";
		constexpr wchar_t QamilStumpShakeNegativeZTriggerId[] = L"Qamil.StumpShake.NegativeZ";
	}

	/// QamilStumpState /////////////////////////////////////////////////////////////////////////////////////////////////
	void QamilStumpState::Enter(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, QamilAnimationId::Stump, false), "Qamil Stump Animation 재생에 실패했습니다.");
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip && clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart), "Qamil Stump Animation에 HitStart Notify가 없습니다.");
		context.animatorComponent->GetNotifyEvent().Subscribe(_notifyConnection, [this, &context](const AnimationNotifyEvent& event) { HandleAnimationNotify(context, event); });
	}

	void QamilStumpState::Tick(QamilStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(QamilStateId::Idle);
	}

	void QamilStumpState::Exit(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	void QamilStumpState::HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::HitStart)
		{
			GM_ASSERT(SpawnHitBox(context), "Qamil Stump HitBox 생성에 실패했습니다.");
			GM_ASSERT(PulseFloorShake(context), "Qamil Stump 바닥 흔들림 실행에 실패했습니다.");
		}
	}

	bool QamilStumpState::SpawnHitBox(QamilStateContext& context) const
	{
		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		if (scene == nullptr || context.transformComponent == nullptr)
			return false;

		TemporaryBoxHitBoxDesc desc{};
		const Vector3 arenaCenter = context.transformComponent->GetPosition();
		desc.world = Matrix::CreateTranslation(arenaCenter);
		desc.colliderId = L"Qamil.Stump";
		desc.size = QamilStumpHitBoxSize;
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = QamilWideAttackDamage;
		desc.damageInfo.hitReactionType = HitReactionType::Airborne;
		desc.damageInfo.worldImpulse = Vector3{ 0.f, QamilStumpVerticalImpulse, 0.f };
		desc.lifetime = QamilWideAttackHitBoxLifetime;
		desc.hitCondition = [groundHeight = arenaCenter.y](const HurtBoxComponent& hurtBox)
		{
			const TransformComponent* targetTransform = hurtBox.GetOwner().GetTransform();
			return targetTransform && targetTransform->GetPosition().y <= groundHeight + QamilStumpGroundTolerance;
		};
		return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
	}

	bool QamilStumpState::PulseFloorShake(QamilStateContext& context) const
	{
		if (context.triggerSystem == nullptr || context.transformComponent == nullptr)
			return false;

		const Vector3 platformOffset = GetCurrentPlatformPosition(context) - context.transformComponent->GetPosition();
		const wchar_t* triggerId = nullptr;
		if (std::abs(platformOffset.x) > std::abs(platformOffset.z))
			triggerId = platformOffset.x > 0.f ? QamilStumpShakeNegativeZTriggerId : QamilStumpShakePositiveZTriggerId;
		else
			triggerId = platformOffset.z > 0.f ? QamilStumpShakePositiveXTriggerId : QamilStumpShakeNegativeXTriggerId;

		return context.triggerSystem->Pulse(triggerId);
	}

	/// QamilSweepState /////////////////////////////////////////////////////////////////////////////////////////////////
	void QamilSweepState::Enter(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		_isClockwise = SelectClockwiseDirection(context);
		const Vector3 arenaCenter = context.transformComponent->GetPosition();
		const Vector3 currentPlatformOffset = GetCurrentPlatformPosition(context) - arenaCenter;
		const Vector3 nextPlatformOffset = Vector3::TransformNormal(currentPlatformOffset, Matrix::CreateRotationY(_isClockwise ? QamilPlatformRotation : -QamilPlatformRotation));
		_attackCenter = arenaCenter + currentPlatformOffset + nextPlatformOffset;

		GM_ASSERT_RETURN(PlayBeatSyncedAnimation(context, _isClockwise ? QamilAnimationId::SweepLeft : QamilAnimationId::SweepRight, false), "Qamil Sweep Animation 재생에 실패했습니다.");
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip && clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart), "Qamil Sweep Animation에 HitStart Notify가 없습니다.");
		context.animatorComponent->GetNotifyEvent().Subscribe(_notifyConnection, [this, &context](const AnimationNotifyEvent& event) { HandleAnimationNotify(context, event); });
	}

	void QamilSweepState::Tick(QamilStateContext& context, float)
	{
		if (IsAnimationCompleted(context) == false)
			return;

		const Vector3 lookDirection = Math::GetLookVector(context.transformComponent->GetRotation());
		const float currentRotationY = std::atan2(lookDirection.x, lookDirection.z);
		context.transformComponent->SetRotationY(currentRotationY + (_isClockwise ? QamilPlatformRotation : -QamilPlatformRotation));
		context.stateMachine->ChangeState(QamilStateId::Idle);
	}

	void QamilSweepState::Exit(QamilStateContext& context)
	{
		_notifyConnection.Disconnect();
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	void QamilSweepState::HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::HitStart)
			GM_ASSERT(SpawnHitBox(context), "Qamil Sweep HitBox 생성에 실패했습니다.");
	}

	bool QamilSweepState::SpawnHitBox(QamilStateContext& context) const
	{
		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		if (scene == nullptr)
			return false;

		TemporarySphereHitBoxDesc desc{};
		desc.world = Matrix::CreateTranslation(_attackCenter);
		desc.colliderId = L"Qamil.Sweep";
		desc.radius = QamilSweepRadius;
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = QamilWideAttackDamage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.lifetime = QamilWideAttackHitBoxLifetime;
		return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
	}
}
