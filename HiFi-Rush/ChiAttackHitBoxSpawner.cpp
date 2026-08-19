#include "ChiAttackHitBoxSpawner.h"
#include "ChiAnimationTypes.h"
#include "ChiStateContext.h"
#include "ChiStateMachineComponent.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "ReverbComponent.h"
#include "Scene.h"
#include "TemporaryBoxHitBoxObject.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		enum class ActivationTriggerType
		{
			HitStartNotify,
			GroundContact,
		};

		struct HitBoxSetting
		{
			Vector3					localCenter{};
			Vector3					size{ 1.f, 1.f, 1.f };
			int32					damage = 0;
			HitReactionType			hitReactionType = HitReactionType::None;
			float					lifetime = 1.05f;
			float					rehitInterval = 0.f;
			ActivationTriggerType	activationTriggerType = ActivationTriggerType::HitStartNotify;
		};

		const HitBoxSetting* FindSetting(ChiAnimationClipId animationClipId)
		{
			static const HitBoxSetting WeakAttackHitBox{ Vector3{ 0.f, 0.75f, 0.6f }, Vector3{ 1.4f, 1.5f, 1.6f }, 10, HitReactionType::WeakKnockback };
			static const HitBoxSetting WeakDashAttackHitBox{ Vector3{ 0.f, 0.75f, 0.9f }, Vector3{ 1.4f, 1.5f, 2.8f }, 10, HitReactionType::WeakKnockback };
			static const HitBoxSetting StrongAttackHitBox{ Vector3{ 0.f, 0.75f, 1.1f }, Vector3{ 2.4f, 1.5f, 2.2f }, 20, HitReactionType::StrongKnockback };
			static const HitBoxSetting AirborneAttackHitBox{ Vector3{ 0.f, 0.75f, 1.1f }, Vector3{ 1.6f, 1.5f, 1.6f }, 10, HitReactionType::Airborne };
			static const HitBoxSetting StrongDashAttackHitBox{ Vector3{ 0.f, 0.75f, 1.1f }, Vector3{ 1.6f, 1.5f, 1.6f }, 20, HitReactionType::Airborne };
			static const HitBoxSetting DelayedWeakAttackHitBox{ Vector3{ 0.f, 0.75f, 1.f }, Vector3{ 1.4f, 1.5f, 2.4f }, 5, HitReactionType::StrongKnockback, 0.2f, 0.1f };
			static const HitBoxSetting AirAttackHitBox{ Vector3{ 0.f, 0.75f, 1.1f }, Vector3{ 1.6f, 2.f, 1.6f }, 10, HitReactionType::Sky };
			static const HitBoxSetting StumpAttackHitBox{ Vector3{ 0.f, 0.75f, 0.f }, Vector3{ 3.f, 1.5f, 3.f }, 20, HitReactionType::StrongKnockback, 1.05f, 0.f, ActivationTriggerType::GroundContact };

			switch (animationClipId)
			{
			case ChiAnimationClipId::AttackWeakDash:
				return &WeakDashAttackHitBox;

			case ChiAnimationClipId::AttackWeak0:
			case ChiAnimationClipId::AttackWeak1:
			case ChiAnimationClipId::AttackWeak2:
			case ChiAnimationClipId::AttackWeak3:
				return &WeakAttackHitBox;

			case ChiAnimationClipId::AttackStrong0_1:
			case ChiAnimationClipId::AttackStrong1:
			case ChiAnimationClipId::AttackStrong2:
			case ChiAnimationClipId::AttackStrongToWeak1:
			case ChiAnimationClipId::AttackWeakToStrong1:
				return &StrongAttackHitBox;

			case ChiAnimationClipId::AttackStrongDash:
				return &StrongDashAttackHitBox;

			case ChiAnimationClipId::AttackDelayedWeak1:
				return &DelayedWeakAttackHitBox;

			case ChiAnimationClipId::AttackDelayedWeak2:
				return &AirborneAttackHitBox;

			case ChiAnimationClipId::AttackSky0:
			case ChiAnimationClipId::AttackSky1:
			case ChiAnimationClipId::AttackSky2:
			case ChiAnimationClipId::AttackSky3:
				return &AirAttackHitBox;

			case ChiAnimationClipId::AttackStump1:
				return &StumpAttackHitBox;

			default:
				return nullptr;
			}
		}

		bool SpawnHitBox(ChiStateContext& context, const HitBoxSetting& setting)
		{
			GameObject& owner = context.stateMachine->GetOwner();
			Scene* scene = owner.GetScene();
			TransformComponent* transform = owner.GetTransform();
			GM_ASSERT_RETURN_VAL(scene && transform, false, "Chi Attack HitBox 생성에 필요한 Scene 또는 Transform이 없습니다.");
			ReverbComponent* reverbComponent = owner.GetComponent<ReverbComponent>();
			GM_ASSERT_RETURN_VAL(reverbComponent, false, "Chi Attack HitBox의 적중 보상을 처리하려면 ReverbComponent가 필요합니다.");

			TemporaryBoxHitBoxDesc desc{};
			desc.world = transform->GetWorldMatrix();
			desc.colliderId = L"Attack";
			desc.localCenter = setting.localCenter;
			desc.size = setting.size;
			desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
			desc.collisionMask = HiFiRushCollisionMask::PlayerAttackTargets;
			desc.damageInfo.amount = setting.damage;
			desc.damageInfo.hitReactionType = setting.hitReactionType;
			desc.onHit = [reverbComponent](const HitEvent& event)
			{
				reverbComponent->HandleAttackHit(event);
			};
			desc.rehitInterval = setting.rehitInterval;
			desc.lifetime = setting.lifetime;
			return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
		}
	}

	bool ChiAttackHitBoxSpawner::IsSpawnedByHitStartNotify(ChiAnimationClipId animationClipId)
	{
		const HitBoxSetting* setting = FindSetting(animationClipId);
		return setting != nullptr && setting->activationTriggerType == ActivationTriggerType::HitStartNotify;
	}

	bool ChiAttackHitBoxSpawner::SpawnForAnimation(ChiStateContext& context, ChiAnimationClipId animationClipId)
	{
		const HitBoxSetting* setting = FindSetting(animationClipId);
		GM_ASSERT_RETURN_VAL(setting, false, "Chi Attack HitBox 설정을 찾을 수 없습니다.");
		return SpawnHitBox(context, *setting);
	}
}
