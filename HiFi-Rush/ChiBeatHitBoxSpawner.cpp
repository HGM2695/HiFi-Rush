#include "ChiBeatHitBoxSpawner.h"
#include "AudioStatics.h"
#include "BeatHitTypes.h"
#include "BeatSystem.h"
#include "ChiAnimationTypes.h"
#include "ChiEffectComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateContext.h"
#include "ChiStateMachineComponent.h"
#include "GameObject.h"
#include "GameplayAnnouncementWidget.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushAudio.h"
#include "MathUtil.h"
#include "ReverbComponent.h"
#include "Scene.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <optional>

namespace gm
{
	namespace
	{
		enum class HitBoxShape
		{
			Box,
			Sphere,
		};

		struct HitBoxSetting
		{
			Vector3			originOffset{};
			float			directionYawDegrees = 0.f;
			float			distance = 0.f;
			Vector3			size{ 1.f, 1.f, 1.f };
			int32			damage = 0;
			HitReactionType hitReactionType = HitReactionType::None;
			float			activationDelayBeats = 0.f;
			HitBoxShape		hitBoxShape = HitBoxShape::Box;
			std::optional<Vector3> effectOriginOffset{};
		};

		bool SpawnHitBox(ChiStateContext& context, const HitBoxSetting& setting, float targetBeat, const wchar_t* activationSoundKey, std::optional<ChiAnimationClipId> activationEffectClipId)
		{
			GameObject& owner = context.stateMachine->GetOwner();
			Scene* scene = owner.GetScene();
			TransformComponent* transform = owner.GetTransform();
			GM_ASSERT_RETURN_VAL(scene && transform, false, "Chi Beat HitBox 생성에 필요한 Scene 또는 Transform이 없습니다.");
			ReverbComponent* reverbComponent = owner.GetComponent<ReverbComponent>();
			GM_ASSERT_RETURN_VAL(reverbComponent, false, "Chi Beat HitBox의 적중 보상을 처리하려면 ReverbComponent가 필요합니다.");

			const Vector3 forward = context.moveComponent->GetForwardDirection();
			const Vector3 right = context.moveComponent->GetRightDirection();
			const Quaternion yawRotation = Quaternion::CreateFromAxisAngle(Vector3::Up, Math::DegreesToRadians(setting.directionYawDegrees));
			const Vector3 direction = Vector3::Transform(forward, yawRotation);
			const Vector3 center = transform->GetPosition() + right * setting.originOffset.x + Vector3::Up * setting.originOffset.y + forward * setting.originOffset.z + direction * setting.distance;
			const Vector3 effectPosition = setting.effectOriginOffset.has_value() ? transform->GetPosition() + right * setting.effectOriginOffset->x + Vector3::Up * setting.effectOriginOffset->y + forward * setting.effectOriginOffset->z : transform->GetPosition() + right * setting.originOffset.x + forward * setting.originOffset.z + direction * setting.distance;
			const Matrix effectWorld = Math::CreateTransformMatrix(effectPosition, Math::CreateRotationByDirection(direction));

			auto configureHitBox = [&setting, reverbComponent, targetBeat, activationSoundKey, activationEffectClipId, effectWorld, &context](TemporaryHitBoxDesc& desc)
			{
				desc.colliderId = L"BeatHitAttack";
				desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
				desc.collisionMask = HiFiRushCollisionMask::PlayerAttackTargets;
				desc.damageInfo.amount = setting.damage;
				desc.damageInfo.hitReactionType = setting.hitReactionType;
				if (activationSoundKey || activationEffectClipId)
				{
					ChiEffectComponent* effectComponent = context.effectComponent;
					desc.onActivated = [activationSoundKey, activationEffectClipId, effectComponent, effectWorld]()
					{
						if (activationSoundKey)
							PlaySound2D(activationSoundKey);
						if (activationEffectClipId && effectComponent)
							effectComponent->SpawnBeatHitEffect(activationEffectClipId.value(), effectWorld);
					};
				}
				desc.onHit = [reverbComponent](const HitEvent& event) { reverbComponent->HandleAttackHit(event); };
				desc.activationDelayBeats = setting.activationDelayBeats;
				if (context.beatSystem->HasPlaybackTime())
					desc.activationDelayBeats += std::max(targetBeat - context.beatSystem->GetCurrentBeat(), 0.f);
				desc.lifetime = 0.05f;
			};

			if (setting.hitBoxShape == HitBoxShape::Sphere)
			{
				TemporarySphereHitBoxDesc desc{};
				desc.world = Matrix::CreateTranslation(center);
				desc.radius = std::max(setting.size.x, setting.size.z) * 0.5f;
				configureHitBox(desc);
				return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
			}

			TemporaryBoxHitBoxDesc desc{};
			desc.world = Math::CreateTransformMatrix(center, Math::CreateRotationByDirection(direction));
			desc.size = setting.size;
			configureHitBox(desc);
			desc.damageInfo.worldKnockbackDirection = forward;
			return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
		}

		template <size_t Size>
		bool SpawnHitBoxes(ChiStateContext& context, const std::array<HitBoxSetting, Size>& settings, float targetBeat, const wchar_t* activationSoundKey = nullptr, std::optional<ChiAnimationClipId> activationEffectClipId = std::nullopt)
		{
			for (const HitBoxSetting& setting : settings)
			{
				if (SpawnHitBox(context, setting, targetBeat, activationSoundKey, activationEffectClipId) == false)
					return false;
			}
			return true;
		}

		bool SpawnWeakToStrongHitBoxes(ChiStateContext& context, const std::array<HitBoxSetting, 4>& settings, float targetBeat, ChiAnimationClipId animationClipId)
		{
			for (size_t index = 0; index < settings.size(); ++index)
			{
				const wchar_t* activationSoundKey = index == 0 ? HiFiRushSound::ChiLeftBranchBeatHit : nullptr;
				if (SpawnHitBox(context, settings[index], targetBeat, activationSoundKey, animationClipId) == false)
					return false;
			}
			return true;
		}

		bool SpawnSuccessHitBoxesForAnimation(ChiStateContext& context, ChiAnimationClipId animationClipId, float targetBeat)
		{
			static const std::array<HitBoxSetting, 2> Weak3HitBoxes =
			{
				HitBoxSetting{ Vector3{ 0.f, 2.f, 0.f }, 0.f, 2.3f, Vector3{ 3.f, 4.f, 3.f }, 15, HitReactionType::StrongKnockback, 1.f },
				HitBoxSetting{ Vector3{ 0.f, 2.f, 0.f }, 0.f, 4.6f, Vector3{ 3.f, 4.f, 3.f }, 15, HitReactionType::StrongKnockback, 2.f },
			};

			static const std::array<HitBoxSetting, 1> Strong2HitBoxes =
			{
				HitBoxSetting{ Vector3{ 0.f, 0.75f, 0.f }, 0.f, 1.1f, Vector3{ 2.4f, 1.5f, 2.2f }, 30, HitReactionType::StrongKnockback },
			};

			static const std::array<HitBoxSetting, 1> StrongToWeak2HitBoxes =
			{
				HitBoxSetting{ Vector3{ 0.f, 1.5f, 1.5f }, 0.f, 8.f, Vector3{ 2.f, 1.5f, 16.f }, 15, HitReactionType::StrongKnockback, 1.f, HitBoxShape::Box, Vector3::Zero },
			};

			static const std::array<HitBoxSetting, 4> WeakToStrong2HitBoxes =
			{
				HitBoxSetting{ Vector3{ 0.6f, 1.f, 0.5f }, 40.f, 7.5f, Vector3{ 2.f, 1.5f, 16.f }, 15, HitReactionType::StrongKnockback, 1.f, HitBoxShape::Box, Vector3{ 0.6f, 1.f, 0.5f } },
				HitBoxSetting{ Vector3{ 0.4f, 1.f, 0.5f }, 10.f, 7.5f, Vector3{ 2.f, 1.5f, 16.f }, 15, HitReactionType::StrongKnockback, 2.f, HitBoxShape::Box, Vector3{ 0.4f, 1.f, 0.5f } },
				HitBoxSetting{ Vector3{ -0.5f, 1.f, 0.5f }, -20.f, 7.5f, Vector3{ 2.f, 1.5f, 16.f }, 15, HitReactionType::StrongKnockback, 3.f, HitBoxShape::Box, Vector3{ -0.5f, 1.f, 0.5f } },
				HitBoxSetting{ Vector3{ -1.f, 1.f, 0.5f }, -55.f, 7.5f, Vector3{ 2.f, 1.5f, 16.f }, 15, HitReactionType::StrongKnockback, 3.25f, HitBoxShape::Box, Vector3{ -1.f, 1.f, 0.5f } },
			};

			static const std::array<HitBoxSetting, 1> HibikiHitBoxes =
			{
				HitBoxSetting{ Vector3{ 0.f, 1.f, 0.f }, 0.f, 0.f, Vector3{ 40.f, 10.f, 40.f }, 50, HitReactionType::Airborne, 0.f, HitBoxShape::Sphere },
			};

			switch (animationClipId)
			{
			case ChiAnimationClipId::AttackWeak3:
				return SpawnHitBoxes(context, Weak3HitBoxes, targetBeat, HiFiRushSound::ChiWeakBeatHit, animationClipId);
			case ChiAnimationClipId::AttackStrong2:
				return SpawnHitBoxes(context, Strong2HitBoxes, targetBeat, HiFiRushSound::ChiStrongBeatHit, animationClipId);
			case ChiAnimationClipId::AttackStrongToWeak2:
				return SpawnHitBoxes(context, StrongToWeak2HitBoxes, targetBeat, nullptr, animationClipId);
			case ChiAnimationClipId::AttackWeakToStrong2:
				return SpawnWeakToStrongHitBoxes(context, WeakToStrong2HitBoxes, targetBeat, animationClipId);
			case ChiAnimationClipId::HibikiAttack:
				return SpawnHitBoxes(context, HibikiHitBoxes, targetBeat);
			default:
				return false;
			}
		}

		void PlaySuccessSound(ChiAnimationClipId animationClipId)
		{
			switch (animationClipId)
			{
			case ChiAnimationClipId::AttackStrongToWeak2:
				PlaySound2D(HiFiRushSound::ChiRightBranchBeatHit);
				break;
			case ChiAnimationClipId::HibikiAttack:
				PlaySound2D(HiFiRushSound::ChiHibikiBeatHit);
				break;
			default:
				break;
			}
		}

		void SpawnSuccessEffect(ChiStateContext& context, ChiAnimationClipId animationClipId)
		{
			if (animationClipId != ChiAnimationClipId::HibikiAttack)
				return;

			context.effectComponent->SpawnBeatHitEffect(animationClipId, context.stateMachine->GetOwner().GetTransform()->GetWorldMatrix());
		}

		void PlaySuccessAnnouncement(ChiStateContext& context, ChiAnimationClipId animationClipId)
		{
			if (animationClipId != ChiAnimationClipId::HibikiAttack)
				return;

			GameplayScene* gameplayScene = dynamic_cast<GameplayScene*>(context.stateMachine->GetOwner().GetScene());
			if (gameplayScene)
				gameplayScene->PlayAnnouncement(GameplayAnnouncementType::Yeah);
		}
	}

	bool ChiBeatHitBoxSpawner::Spawn(ChiStateContext& context, ChiAnimationClipId animationClipId, const BeatHitResultEvent& resultEvent)
	{
		if (resultEvent.result != BeatHitResult::Success)
			return true;

		if (SpawnSuccessHitBoxesForAnimation(context, animationClipId, resultEvent.targetBeat) == false)
			return false;
		PlaySuccessSound(animationClipId);
		SpawnSuccessEffect(context, animationClipId);
		PlaySuccessAnnouncement(context, animationClipId);
		return true;
	}
}
