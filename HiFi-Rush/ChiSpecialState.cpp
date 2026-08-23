#include "ChiSpecialState.h"
#include "AudioStatics.h"
#include "BeatSystem.h"
#include "CameraComponent.h"
#include "CameraFollowComponent.h"
#include "CameraManager.h"
#include "ChiEffectComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushAudio.h"
#include "Scene.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	ChiHibikiReadyState::ChiHibikiReadyState()
		: ChiState(ChiStateId::HibikiReady, ChiAnimationClipId::HibikiReady)
	{
	}

	void ChiHibikiReadyState::Enter(ChiStateContext& context)
	{
		ChiState::Enter(context);
		context.healthComponent->SetInvincible(true);
		PlaySound2D(HiFiRushSound::ChiHibikiReady);
	}

	void ChiHibikiReadyState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::HibikiAttack);
	}

	void ChiHibikiReadyState::Exit(ChiStateContext& context)
	{
		context.healthComponent->SetInvincible(false);
	}

	ChiHibikiAttackState::ChiHibikiAttackState()
		: ChiAttackState(ChiStateId::HibikiAttack, ChiAnimationClipId::HibikiAttack)
	{
	}

	void ChiHibikiAttackState::Enter(ChiStateContext& context)
	{
		ChiAttackState::Enter(context);
		context.healthComponent->SetInvincible(true);
		_cameraFollowComponent = FindCameraFollowComponent(context);
		_previousCameraTargetDistance.reset();
		if (_cameraFollowComponent)
		{
			_previousCameraTargetDistance = _cameraFollowComponent->GetTargetDistance();
			_cameraFollowComponent->SetTargetDistance(6.f);
		}
		_hasStartedCameraMove = false;
		_hasSpawnedAreaHitBox = false;
		_hasSpawnedExitClouds = false;
	}

	void ChiHibikiAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		constexpr float AnimationTicksPerBeat = 15.f;
		constexpr float AreaHitBoxStartBeat = 1.f + 2.f / AnimationTicksPerBeat;
		constexpr float HibikiCloudEndBeat = 3.f + 5.f / AnimationTicksPerBeat;
		constexpr float CameraMoveStartBeat = 6.f;
		constexpr float CameraMoveDurationBeats = 2.f;
		constexpr float HibikiExitCloudBeat = 8.f;
		const float animationBeat = GetAnimationBeat(context);
		if (_hasSpawnedAreaHitBox == false && animationBeat >= AreaHitBoxStartBeat)
		{
			_hasSpawnedAreaHitBox = true;
			GM_ASSERT(SpawnAreaHitBox(context), "히비키 광역 HitBox GameObject 생성에 실패했습니다.");
			PlaySound2D(HiFiRushSound::ChiHibikiAttack, 0.5f);
			Scene* scene = context.stateMachine->GetOwner().GetScene();
			if (scene && scene->GetCameraManager())
			{
				CameraShakeDesc shakeDesc{};
				shakeDesc.duration = 2.f;
				shakeDesc.strength = 0.15f;
				shakeDesc.locationAmplitude.y = 0.f;
				scene->GetCameraManager()->AddShake(shakeDesc);
			}
		}
		if (animationBeat >= AreaHitBoxStartBeat && animationBeat < HibikiCloudEndBeat)
			context.effectComponent->SpawnHibikiCloud();
		if (_hasStartedCameraMove == false && animationBeat >= CameraMoveStartBeat && _cameraFollowComponent)
		{
			_hasStartedCameraMove = true;
			const Vector3 targetPosition = _cameraFollowComponent->GetFollowTargetPosition() - context.moveComponent->GetForwardDirection() * 4.f - Vector3::Up * 0.5f;
			const float duration = CameraMoveDurationBeats * context.beatSystem->GetSecondsPerBeat();
			_cameraFollowComponent->StartWorldPositionMove(targetPosition, duration);
		}
		if (_hasSpawnedExitClouds == false && animationBeat >= HibikiExitCloudBeat)
		{
			_hasSpawnedExitClouds = true;
			context.effectComponent->SpawnHibikiExitClouds();
			RestoreCamera();
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	void ChiHibikiAttackState::Exit(ChiStateContext& context)
	{
		RestoreCamera();
		context.healthComponent->SetInvincible(false);
		ChiAttackState::Exit(context);
	}

	CameraFollowComponent* ChiHibikiAttackState::FindCameraFollowComponent(ChiStateContext& context) const
	{
		Scene* scene = context.stateMachine->GetOwner().GetScene();
		CameraManager* cameraManager = scene ? scene->GetCameraManager() : nullptr;
		CameraComponent* activeCamera = cameraManager ? cameraManager->GetActiveCamera() : nullptr;
		return activeCamera ? activeCamera->GetOwner().GetComponent<CameraFollowComponent>() : nullptr;
	}

	void ChiHibikiAttackState::RestoreCamera()
	{
		if (_cameraFollowComponent == nullptr)
			return;

		_cameraFollowComponent->StopWorldPositionMove();
		if (_previousCameraTargetDistance)
			_cameraFollowComponent->SetTargetDistance(_previousCameraTargetDistance.value());
		_cameraFollowComponent = nullptr;
		_previousCameraTargetDistance.reset();
	}

	bool ChiHibikiAttackState::SpawnAreaHitBox(ChiStateContext& context) const
	{
		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		TransformComponent* transform = owner.GetTransform();
		GM_ASSERT_RETURN_VAL(scene && transform, false, "히비키 광역 HitBox 생성에 필요한 Scene 또는 Transform이 없습니다.");

		TemporarySphereHitBoxDesc desc{};
		desc.world = transform->GetWorldMatrix();
		desc.colliderId = L"HibikiAttack";
		desc.localCenter = Vector3{ 0.f, 2.5f, 0.f };
		desc.radius = 20.f;
		desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
		desc.collisionMask = HiFiRushCollisionMask::PlayerAttackTargets;
		desc.damageInfo.amount = 3;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.rehitInterval = 0.1f;
		desc.lifetime = 1.f;
		return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
	}
}
