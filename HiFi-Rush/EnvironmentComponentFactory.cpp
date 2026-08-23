#include "EnvironmentComponentFactory.h"
#include "BeatAudioLevelMoveComponent.h"
#include "BeatMaterialBrightnessPulseComponent.h"
#include "BeatMoveComponent.h"
#include "BeatOrbitComponent.h"
#include "BeatPositionSequenceComponent.h"
#include "BeatSkeletalAnimationSyncComponent.h"
#include "BeatStaticMeshCycleComponent.h"
#include "BeatTextureSequenceComponent.h"
#include "BeatTextureUVScrollComponent.h"
#include "BeatTextureUVStepComponent.h"
#include "BeatTransformComponent.h"
#include "BeatTriggeredRotationComponent.h"
#include "BeatTriggeredRotationShakeComponent.h"
#include "BeatTriggeredSkeletalAnimationComponent.h"
#include "BeatVisibilityComponent.h"
#include "BoxCollider3DComponent.h"
#include "Collider3DComponent.h"
#include "CollisionTriggerComponent.h"
#include "ContinuousRotationComponent.h"
#include "DirectionalLightComponent.h"
#include "FallRespawnTriggerComponent.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "HealthComponent.h"
#include "HitReactionComponent.h"
#include "HurtBoxComponent.h"
#include "MovementBaseComponent.h"
#include "MathUtil.h"
#include "PointLightComponent.h"
#include "Resources.h"
#include "RespawnPointTriggerComponent.h"
#include "SceneTransitionTriggerComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "SphereCollider3DComponent.h"
#include "SpotLightComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"
#include "TriggeredLightColorComponent.h"
#include "TriggeredMaterialOverrideComponent.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace gm
{
	EnvironmentComponentFactory::EnvironmentComponentFactory(Resources& resources, const BeatSystem& beatSystem)
		: _resources(resources), _beatSystem(beatSystem)
	{}

	bool EnvironmentComponentFactory::AddComponents(GameObject& gameObject, const std::vector<EnvironmentComponentData>& components) const
	{
		for (const EnvironmentComponentData& component : components)
		{
			const bool succeeded = std::visit(
				[this, &gameObject](const auto& data)
				{
					return CreateComponent(gameObject, data);
				}, component);

			if (succeeded == false)
				return false;
		}

		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BoxCollider3DComponentData& data) const
	{
		const std::vector<Collider3DComponent*>& colliders = gameObject.GetColliders3D();
		const bool hasDuplicateId = data.colliderId.empty() == false && std::any_of(colliders.begin(), colliders.end(),
			[&data](const Collider3DComponent* collider)
			{
				return collider->GetColliderId() == data.colliderId;
			});
		GM_ASSERT_RETURN_VAL(hasDuplicateId == false, false, "환경 GameObject에 중복된 Collider ID가 있습니다. colliderId=%ls", data.colliderId.c_str());
		GM_ASSERT_RETURN_VAL(data.size.x > 0.f && data.size.y > 0.f && data.size.z > 0.f, false, "BoxCollider3D의 크기는 모든 축에서 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(IsSingleCollisionLayer(data.collisionFilter.layer), false, "BoxCollider3D의 Collision Layer는 하나의 비트만 사용해야 합니다.");
		GM_ASSERT_RETURN_VAL(data.localRotation.LengthSquared() > 0.000001f, false, "BoxCollider3D의 Local Rotation은 유효한 Quaternion이어야 합니다.");

		BoxCollider3DComponent* collider = gameObject.AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(collider, false, "BoxCollider3DComponent 생성에 실패했습니다.");
		Quaternion localRotation = data.localRotation;
		localRotation.Normalize();
		collider->SetColliderId(data.colliderId);
		collider->SetLocalCenter(data.localCenter);
		collider->SetLocalRotation(localRotation);
		collider->SetSize(data.size);
		collider->SetCollisionFilter(data.collisionFilter);
		collider->SetTrigger(data.isTrigger);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const SphereCollider3DComponentData& data) const
	{
		const std::vector<Collider3DComponent*>& colliders = gameObject.GetColliders3D();
		const bool hasDuplicateId = data.colliderId.empty() == false && std::any_of(colliders.begin(), colliders.end(),
			[&data](const Collider3DComponent* collider)
			{
				return collider->GetColliderId() == data.colliderId;
			});
		GM_ASSERT_RETURN_VAL(hasDuplicateId == false, false, "환경 GameObject에 중복된 Collider ID가 있습니다. colliderId=%ls", data.colliderId.c_str());
		GM_ASSERT_RETURN_VAL(std::isfinite(data.radius) && data.radius > 0.f, false, "SphereCollider3D의 반지름은 0보다 큰 유한한 값이어야 합니다.");
		GM_ASSERT_RETURN_VAL(IsSingleCollisionLayer(data.collisionFilter.layer), false, "SphereCollider3D의 Collision Layer는 하나의 비트만 사용해야 합니다.");

		SphereCollider3DComponent* collider = gameObject.AddComponent<SphereCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(collider, false, "SphereCollider3DComponent 생성에 실패했습니다.");
		collider->SetColliderId(data.colliderId);
		collider->SetLocalCenter(data.localCenter);
		collider->SetRadius(data.radius);
		collider->SetCollisionFilter(data.collisionFilter);
		collider->SetTrigger(data.isTrigger);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const HealthComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<HealthComponent>() == nullptr, false, "환경 GameObject에는 HealthComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.maxHealth > 0, false, "Health의 Max Health는 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(std::isfinite(data.damageInvincibilityDuration) && data.damageInvincibilityDuration >= 0.f, false, "Health의 피격 무적 시간은 0 이상의 유한한 값이어야 합니다.");

		HealthComponent* health = gameObject.AddComponent<HealthComponent>(data.maxHealth);
		GM_ASSERT_RETURN_VAL(health, false, "HealthComponent 생성에 실패했습니다.");
		health->SetDamageInvincibilityDuration(data.damageInvincibilityDuration);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const HurtBoxComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "HurtBox가 참조할 Collider ID는 비어 있을 수 없습니다.");
		bool hasDuplicateReference = false;
		gameObject.ForEachComponent(
			[&data, &hasDuplicateReference](const Component& component)
			{
				const auto* hurtBox = dynamic_cast<const HurtBoxComponent*>(&component);
				if (hurtBox != nullptr && hurtBox->GetColliderId() == data.colliderId)
					hasDuplicateReference = true;
			});
		GM_ASSERT_RETURN_VAL(hasDuplicateReference == false, false, "하나의 Collider에는 하나의 HurtBox만 연결할 수 있습니다. colliderId=%ls", data.colliderId.c_str());
		return gameObject.AddComponent<HurtBoxComponent>(data.colliderId) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const HitReactionComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<HitReactionComponent>() == nullptr, false, "GameObject에는 HitReactionComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.completionTriggerId.empty() == false, false, "HitReaction의 완료 Trigger ID가 비어 있습니다.");

		HitReactionComponent* component = gameObject.AddComponent<HitReactionComponent>(data.completionTriggerId, data.reactionAnimationClipName);
		GM_ASSERT_RETURN_VAL(component, false, "HitReactionComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const SceneTransitionTriggerComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<SceneTransitionTriggerComponent>() == nullptr, false, "GameObject에는 SceneTransitionTriggerComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "Scene Transition이 참조할 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(data.targetSceneName.empty() == false, false, "Scene Transition의 Target Scene 이름은 비어 있을 수 없습니다.");

		return gameObject.AddComponent<SceneTransitionTriggerComponent>(data.colliderId, data.targetSceneName) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const CollisionTriggerComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<CollisionTriggerComponent>() == nullptr, false, "GameObject에는 CollisionTriggerComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "Collision Trigger가 참조할 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(data.triggerId.empty() == false, false, "Collision Trigger가 실행할 Trigger ID는 비어 있을 수 없습니다.");

		return gameObject.AddComponent<CollisionTriggerComponent>(data.colliderId, data.triggerId) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const RespawnPointTriggerComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<RespawnPointTriggerComponent>() == nullptr, false, "GameObject에는 RespawnPointTriggerComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "Respawn Point Trigger가 참조할 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(std::isfinite(data.respawnPosition.x) && std::isfinite(data.respawnPosition.y) && std::isfinite(data.respawnPosition.z), false, "Respawn Point Trigger의 Respawn Position은 유한한 값이어야 합니다.");
		GM_ASSERT_RETURN_VAL(std::isfinite(data.respawnRotationY), false, "Respawn Point Trigger의 Respawn Rotation Y는 유한한 값이어야 합니다.");

		return gameObject.AddComponent<RespawnPointTriggerComponent>(data.colliderId, data.respawnPosition, data.respawnRotationY) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const MovementBaseComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<MovementBaseComponent>() == nullptr, false, "GameObject에는 MovementBaseComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "Movement Base가 참조할 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(data.passengerMask != 0, false, "Movement Base의 Passenger Mask는 비어 있을 수 없습니다.");

		return gameObject.AddComponent<MovementBaseComponent>(data.colliderId, data.passengerMask) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const FallRespawnTriggerComponentData& data) const
	{
		GM_ASSERT_RETURN_VAL(gameObject.GetComponent<FallRespawnTriggerComponent>() == nullptr, false, "GameObject에는 FallRespawnTriggerComponent를 하나만 추가할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "Fall Respawn Trigger가 참조할 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(data.damage > 0, false, "Fall Respawn Trigger의 Damage는 0보다 커야 합니다.");

		return gameObject.AddComponent<FallRespawnTriggerComponent>(data.colliderId, data.damage) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const DirectionalLightComponentData& data) const
	{
		DirectionalLightComponent* light = gameObject.AddComponent<DirectionalLightComponent>();
		GM_ASSERT_RETURN_VAL(light, false, "DirectionalLightComponent 생성에 실패했습니다.");
		light->SetColor(data.color);
		light->SetIntensity(data.intensity);
		light->SetCastsShadow(data.castsShadow);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const PointLightComponentData& data) const
	{
		PointLightComponent* light = gameObject.AddComponent<PointLightComponent>();
		GM_ASSERT_RETURN_VAL(light, false, "PointLightComponent 생성에 실패했습니다.");
		light->SetColor(data.color);
		light->SetIntensity(data.intensity);
		light->SetRange(data.range);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const SpotLightComponentData& data) const
	{
		SpotLightComponent* light = gameObject.AddComponent<SpotLightComponent>();
		GM_ASSERT_RETURN_VAL(light, false, "SpotLightComponent 생성에 실패했습니다.");
		light->SetColor(data.color);
		light->SetIntensity(data.intensity);
		light->SetRange(data.range);
		light->SetConeRadians(Math::DegreesToRadians(data.innerConeAngleDegrees), Math::DegreesToRadians(data.outerConeAngleDegrees));
		light->SetConeFalloff(data.coneFalloff);
		light->SetCookieEnabled(data.useCookie);
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const ContinuousRotationComponentData& data) const
	{
		ContinuousRotationDesc desc{};
		desc.axis = data.axis;
		desc.angularSpeedDegrees = data.angularSpeedDegrees;
		return gameObject.AddComponent<ContinuousRotationComponent>(desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const TriggeredLightColorComponentData& data) const
	{
		TriggeredLightColorDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.color = data.color;
		return gameObject.AddComponent<TriggeredLightColorComponent>(_beatSystem, std::move(desc)) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTextureUVScrollComponentData& data) const
	{
		BeatTextureUVScrollDesc desc{};
		desc.materialSlot = data.materialSlot;
		desc.offsetPerBeat = data.offsetPerBeat;
		return gameObject.AddComponent<BeatTextureUVScrollComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTextureUVStepComponentData& data) const
	{
		BeatTextureUVStepDesc desc{};
		desc.materialSlot = data.materialSlot;
		desc.firstOffset = data.firstOffset;
		desc.secondOffset = data.secondOffset;
		desc.stepDurationBeats = data.stepDurationBeats;
		return gameObject.AddComponent<BeatTextureUVStepComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatMaterialBrightnessPulseComponentData& data) const
	{
		BeatMaterialBrightnessPulseDesc desc{};
		desc.materialSlot = data.materialSlot;
		desc.activeBeatMask = data.activeBeatMask;
		desc.patternLengthBeats = data.patternLengthBeats;
		desc.minimumBrightness = data.minimumBrightness;
		desc.maximumBrightness = data.maximumBrightness;
		return gameObject.AddComponent<BeatMaterialBrightnessPulseComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatMoveComponentData& data) const
	{
		BeatMoveDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.targetPosition = data.targetPosition;
		desc.durationBeats = data.durationBeats;
		return gameObject.AddComponent<BeatMoveComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTriggeredRotationComponentData& data) const
	{
		BeatTriggeredRotationDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.axis = data.axis;
		desc.angleDegrees = data.angleDegrees;
		desc.durationBeats = data.durationBeats;
		return gameObject.AddComponent<BeatTriggeredRotationComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTriggeredRotationShakeComponentData& data) const
	{
		BeatTriggeredRotationShakeDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.axis = data.axis;
		desc.angleDegrees = data.angleDegrees;
		desc.durationBeats = data.durationBeats;
		return gameObject.AddComponent<BeatTriggeredRotationShakeComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatPositionSequenceComponentData& data) const
	{
		BeatPositionSequenceDesc desc{};
		desc.positionOffsets = data.positionOffsets;
		desc.beatsPerStep = data.beatsPerStep;
		desc.interpolationSpeed = data.interpolationSpeed;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatPositionSequenceComponent>(_beatSystem, desc), false, "BeatPositionSequenceComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatVisibilityComponentData& data) const
	{
		BeatVisibilityDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.initialVisible = data.initialVisible;
		desc.visibleOnTrigger = data.visibleOnTrigger;
		return gameObject.AddComponent<BeatVisibilityComponent>(_beatSystem, desc) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTransformComponentData& data) const
	{
		TransformComponent* transform = gameObject.GetTransform();
		GM_ASSERT_RETURN_VAL(transform, false, "BeatTransformComponent에 필요한 TransformComponent가 없습니다.");
		if (data.overrideInitialScale)
			transform->SetScale(data.initialScale);

		BeatTransformDesc desc{};
		desc.type = data.type == EnvironmentBeatTransformType::PositionOffset ? BeatTransformType::PositionOffset : BeatTransformType::ScaleMultiplier;
		desc.positionOffset = data.positionOffset;
		desc.maxScaleMultiplier = data.maxScaleMultiplier;
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatTransformComponent>(_beatSystem, desc), false, "BeatTransformComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatOrbitComponentData& data) const
	{
		BeatOrbitDesc desc{};
		desc.center = data.center;
		desc.evenBeatDeltaDegrees = data.evenBeatDeltaDegrees;
		desc.oddBeatDeltaDegrees = data.oddBeatDeltaDegrees;
		desc.interpolationSpeed = data.interpolationSpeed;
		desc.faceCenter = data.faceCenter;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatOrbitComponent>(_beatSystem, desc), false, "BeatOrbitComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatAudioLevelMoveComponentData& data) const
	{
		BeatAudioLevelMoveDesc desc{};
		desc.direction = data.direction;
		desc.maxDistance = data.maxDistance;
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatAudioLevelMoveComponent>(_beatSystem, desc), false, "BeatAudioLevelMoveComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatStaticMeshCycleComponentData& data) const
	{
		StaticMeshComponent* meshComponent = gameObject.GetComponent<StaticMeshComponent>();
		GM_ASSERT_RETURN_VAL(meshComponent, false, "BeatStaticMeshCycleComponent에 필요한 StaticMeshComponent가 없습니다.");
		GM_ASSERT_RETURN_VAL(data.modelIndices.empty() == false, false, "StaticMesh 전환에 사용할 모델 목록이 비어 있습니다.");

		std::vector<std::shared_ptr<StaticMesh>> meshVariants;
		meshVariants.reserve(data.modelIndices.size());
		for (uint32 modelIndex : data.modelIndices)
		{
			const std::wstring modelKey = L"Environment" + std::to_wstring(modelIndex);
			std::shared_ptr<StaticMesh> variant = _resources.Find<StaticMesh>(modelKey);
			GM_ASSERT_RETURN_VAL(variant, false, "StaticMesh 변형 리소스가 없습니다. key=%ls", modelKey.c_str());
			meshVariants.push_back(std::move(variant));
		}

		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatStaticMeshCycleComponent>(_beatSystem, *meshComponent, std::move(meshVariants)), false, "BeatStaticMeshCycleComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTextureSequenceComponentData& data) const
	{
		BeatTextureSequenceDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.materialSlot = data.materialSlot;
		desc.textureSlot = data.textureSlot;
		desc.initialTextureKeys = data.initialTextureKeys;
		desc.triggeredTextureKeys = data.triggeredTextureKeys;
		desc.framesPerBeat = data.framesPerBeat;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		return gameObject.AddComponent<BeatTextureSequenceComponent>(_resources, _beatSystem, std::move(desc)) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const TriggeredMaterialOverrideComponentData& data) const
	{
		TriggeredMaterialOverrideDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.overrides.reserve(data.overrides.size());
		for (const MaterialTextureOverrideData& overrideData : data.overrides)
			desc.overrides.push_back({ overrideData.materialSlot, overrideData.textureSlot, overrideData.textureKey });
		return gameObject.AddComponent<TriggeredMaterialOverrideComponent>(_resources, _beatSystem, std::move(desc)) != nullptr;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatSkeletalAnimationSyncComponentData& data) const
	{
		SkeletalAnimatorComponent* animator = gameObject.GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "BeatSkeletalAnimationSyncComponent에 필요한 SkeletalAnimatorComponent가 없습니다.");

		BeatSkeletalAnimationSyncDesc desc{};
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatSkeletalAnimationSyncComponent>(_beatSystem, *animator, desc), false, "BeatSkeletalAnimationSyncComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTriggeredSkeletalAnimationComponentData& data) const
	{
		SkeletalAnimatorComponent* animator = gameObject.GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "BeatTriggeredSkeletalAnimationComponent에 필요한 SkeletalAnimatorComponent가 없습니다.");

		BeatTriggeredSkeletalAnimationDesc desc{};
		desc.triggerId = data.triggerBindingData.triggerId;
		desc.beatOffset = data.triggerBindingData.beatOffset;
		desc.clipName = data.clipName;
		desc.initiallyVisible = data.initiallyVisible;
		desc.hideWhenCompleted = data.hideWhenCompleted;
		desc.disableCollidersWhenCompleted = data.disableCollidersWhenCompleted;
		return gameObject.AddComponent<BeatTriggeredSkeletalAnimationComponent>(_beatSystem, *animator, std::move(desc)) != nullptr;
	}
}
