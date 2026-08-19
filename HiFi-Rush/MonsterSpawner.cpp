#include "MonsterSpawner.h"

#include "BeatSkeletalAnimationSyncComponent.h"
#include "BeatTriggeredSkeletalAnimationComponent.h"
#include "BeatVisibilityComponent.h"
#include "BoxCollider3DComponent.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "GunnerStateMachineComponent.h"
#include "GunnerAnimationTypes.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "HitBoxComponent.h"
#include "HurtBoxComponent.h"
#include "MathUtil.h"
#include "MonsterCombatComponent.h"
#include "MonsterCombatActivationComponent.h"
#include "MonsterResourceInfo.h"
#include "MonsterStateMachineComponent.h"
#include "MonsterTypes.h"
#include "NavMeshControllerComponent.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SjangoStateMachineComponent.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SocketComponent.h"
#include "SocketFollowComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "SwordStateMachineComponent.h"
#include "SwordAnimationTypes.h"
#include "TransformComponent.h"

#include <array>
#include <utility>

namespace gm
{
	namespace
	{
		constexpr float MonsterCollisionResponseWeight = 100.f;

		constexpr std::array<const wchar_t*, 5> BeatSyncedLocomotionClipNames =
		{
			L"ANIM_IDLE",
			L"ANIM_WALK_FRONT",
			L"ANIM_WALK_BACK",
			L"ANIM_WALK_LEFT",
			L"ANIM_WALK_RIGHT",
		};

		std::wstring GetAppearanceAnimationClipName(MonsterType monsterType)
		{
			switch (monsterType)
			{
			case MonsterType::Sword:
				return GetSwordAnimationClipName(SwordAnimationId::Appear);
			case MonsterType::Gunner:
				return GetGunnerAnimationClipName(GunnerAnimationId::Appear);
			default:
				return {};
			}
		}
	}

	MonsterSpawner::MonsterSpawner(Resources& resources)
		: _resources(resources)
	{
	}

	bool MonsterSpawner::SpawnAll(Scene& scene, const std::vector<MonsterSpawnData>& spawnDataList, std::vector<MonsterSpawnResult>& outSpawnResults) const
	{
		outSpawnResults.clear();
		outSpawnResults.reserve(spawnDataList.size());
		for (const MonsterSpawnData& spawnData : spawnDataList)
		{
			GameObject* monster = Spawn(scene, spawnData);
			GM_ASSERT_RETURN_VAL(monster, false, "Monster 생성에 실패했습니다.");
			outSpawnResults.push_back({ monster->GetWeakPtr(), spawnData.activationTriggerId });
		}

		return true;
	}

	GameObject* MonsterSpawner::Spawn(Scene& scene, const MonsterSpawnData& data) const
	{
		const MonsterResourceInfo* resourceInfo = GetMonsterResourceInfo(data.type);
		GM_ASSERT_RETURN_VAL(resourceInfo, nullptr, "지원하지 않는 Monster Type입니다.");

		const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(resourceInfo->commonResourceKey);
		GM_ASSERT_RETURN_VAL(skeletalMesh, nullptr, "Monster SkeletalMesh가 로드되지 않았습니다. key=%ls", resourceInfo->commonResourceKey);

		const std::wstring defaultAnimationKey = GetMonsterDefaultAnimationClipKey(data.type);
		const std::shared_ptr<SkeletalAnimationClip> defaultAnimation = _resources.Find<SkeletalAnimationClip>(defaultAnimationKey);
		GM_ASSERT_RETURN_VAL(defaultAnimation, nullptr, "Monster 기본 Animation이 로드되지 않았습니다. key=%ls", defaultAnimationKey.c_str());

		GameObject* monster = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(monster, nullptr, "Monster GameObject 생성에 실패했습니다.");
		monster->GetTransform()->SetWorldMatrix(data.world);

		SkeletalMeshComponent* meshComponent = monster->AddComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL(meshComponent, nullptr, "Monster SkeletalMeshComponent 생성에 실패했습니다.");
		meshComponent->SetSkeletalMesh(skeletalMesh);

		SkeletalAnimatorComponent* animator = monster->AddComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, nullptr, "Monster SkeletalAnimatorComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(animator->AddClip(L"Default", defaultAnimation), nullptr, "Monster 기본 Animation 등록에 실패했습니다.");

		AnimationPlayOption playOption{};
		playOption.loopOverride = true;
		GM_ASSERT_RETURN_VAL(animator->Play(L"Default", playOption), nullptr, "Monster 기본 Animation 재생에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(AddCommonComponents(*monster, data), nullptr, "Monster 공통 Component 구성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(AddActivationComponents(*monster, data), nullptr, "Monster Trigger 활성화 Component 구성에 실패했습니다.");

		if (data.type == MonsterType::Sword)
		{
			const std::shared_ptr<StaticMesh> weaponMesh = _resources.Find<StaticMesh>(resourceInfo->weaponResourceKey);
			GM_ASSERT_RETURN_VAL(weaponMesh, nullptr, "Sword Weapon StaticMesh가 로드되지 않았습니다.");

			GameObject* weapon = scene.SpawnGameObject<GameObject>();
			GM_ASSERT_RETURN_VAL(weapon, nullptr, "Sword Weapon GameObject 생성에 실패했습니다.");
			StaticMeshComponent* weaponMeshComponent = weapon->AddComponent<StaticMeshComponent>();
			GM_ASSERT_RETURN_VAL(weaponMeshComponent, nullptr, "Sword Weapon StaticMeshComponent 생성에 실패했습니다.");
			weaponMeshComponent->SetStaticMesh(weaponMesh);

			SocketFollowComponent* socketFollow = weapon->AddComponent<SocketFollowComponent>();
			GM_ASSERT_RETURN_VAL(socketFollow, nullptr, "Sword Weapon SocketFollowComponent 생성에 실패했습니다.");
			socketFollow->SetTarget(*monster, L"Sword.Weapon");
			socketFollow->SetDestroyWithTarget(true);

			if (data.playAppearanceAnimation)
			{
				BeatVisibilityDesc visibilityDesc{};
				visibilityDesc.triggerId = data.activationTriggerId;
				visibilityDesc.beatOffset = data.appearanceBeatOffset;
				visibilityDesc.initialVisible = false;
				visibilityDesc.visibleOnTrigger = true;
				GM_ASSERT_RETURN_VAL(weapon->AddComponent<BeatVisibilityComponent>(HiFiRushStatics::GetBeatSystem(), visibilityDesc), nullptr, "Sword Weapon Trigger Visibility Component 생성에 실패했습니다.");
			}
		}

		return monster;
	}

	bool MonsterSpawner::AddCommonComponents(GameObject& monster, const MonsterSpawnData& data) const
	{
		GM_ASSERT_RETURN_VAL(data.maxHealth > 0, false, "Monster Max Health는 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(
			data.bodyColliderSize.x > 0.f && data.bodyColliderSize.y > 0.f && data.bodyColliderSize.z > 0.f,
			false, "Monster Body Collider Size는 모든 축에서 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(data.moveSpeed >= 0.f, false, "Monster Move Speed는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(data.rotationInterpSpeed >= 0.f, false, "Monster Rotation Interp Speed는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(data.attackCooldownBeats >= 0.f, false, "Monster Attack Cooldown은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(data.attackDamage >= 0, false, "Monster Attack Damage는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(data.attackRangeMin >= 0.f, false, "Monster Attack Range Min은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(data.attackRangeMax >= data.attackRangeMin, false, "Monster Attack Range Max는 Min 이상이어야 합니다.");
		if (data.type != MonsterType::Sjango)
		{
			GM_ASSERT_RETURN_VAL(data.attackDamage > 0, false, "Combat Monster Attack Damage는 0보다 커야 합니다.");
			GM_ASSERT_RETURN_VAL(data.attackRangeMax > data.attackRangeMin, false, "Combat Monster Attack Range가 유효하지 않습니다.");
		}

		Rigidbody3DComponent* rigidbody = monster.AddComponent<Rigidbody3DComponent>();
		GM_ASSERT_RETURN_VAL(rigidbody, false, "Monster Rigidbody3DComponent 생성에 실패했습니다.");
		rigidbody->SetGravityScale(3.f);
		rigidbody->SetCollisionResponseWeight(MonsterCollisionResponseWeight);

		BoxCollider3DComponent* bodyCollider = monster.AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(bodyCollider, false, "Monster Body Collider 생성에 실패했습니다.");
		bodyCollider->SetColliderId(L"Body");
		bodyCollider->SetLocalCenter(data.bodyColliderCenter);
		bodyCollider->SetSize(data.bodyColliderSize);
		bodyCollider->SetCollisionLayer(HiFiRushCollisionLayer::Monster);
		bodyCollider->SetCollisionMask(AllCollisionLayers & ~(HiFiRushCollisionLayer::PlayerAttack | HiFiRushCollisionLayer::MonsterAttack));
		bodyCollider->SetCollisionResponseMode(CollisionResponseMode::Planar);

		BoxCollider3DComponent* hurtCollider = monster.AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(hurtCollider, false, "Monster Hurt Collider 생성에 실패했습니다.");
		hurtCollider->SetColliderId(L"HurtBox");
		hurtCollider->SetLocalCenter(data.bodyColliderCenter);
		hurtCollider->SetSize(data.bodyColliderSize);
		hurtCollider->SetCollisionLayer(HiFiRushCollisionLayer::Monster);
		hurtCollider->SetCollisionMask(HiFiRushCollisionLayer::PlayerAttack);

		HealthComponent* health = monster.AddComponent<HealthComponent>(data.maxHealth);
		GM_ASSERT_RETURN_VAL(health, false, "Monster HealthComponent 생성에 실패했습니다.");
		health->SetInvincible(data.isInvincible);
		GM_ASSERT_RETURN_VAL(monster.AddComponent<HurtBoxComponent>(L"HurtBox"), false, "Monster HurtBoxComponent 생성에 실패했습니다.");

		NavMeshControllerComponent* navMeshController = monster.AddComponent<NavMeshControllerComponent>();
		GM_ASSERT_RETURN_VAL(navMeshController, false, "Monster NavMeshControllerComponent 생성에 실패했습니다.");

		GM_ASSERT_RETURN_VAL(monster.AddComponent<CharacterMovementComponent>(data.moveSpeed, data.rotationInterpSpeed), false, "Monster CharacterMovementComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(monster.AddComponent<MonsterCombatComponent>(data.type, HiFiRushStatics::GetBeatSystem(), data.attackCooldownBeats), false, "MonsterCombatComponent 생성에 실패했습니다.");
		if (data.type != MonsterType::Sjango)
		{
			SkeletalAnimatorComponent* animator = monster.GetComponent<SkeletalAnimatorComponent>();
			GM_ASSERT_RETURN_VAL(animator, false, "Combat Monster에 SkeletalAnimatorComponent가 없습니다.");

			BeatSkeletalAnimationSyncDesc animationSyncDesc{};
			BeatSkeletalAnimationSyncComponent* animationSync = monster.AddComponent<BeatSkeletalAnimationSyncComponent>(HiFiRushStatics::GetBeatSystem(), *animator, animationSyncDesc);
			GM_ASSERT_RETURN_VAL(animationSync, false, "Combat Monster BeatSkeletalAnimationSyncComponent 생성에 실패했습니다.");

			const BeatSkeletalAnimationSyncDesc twoBeatSync{ .cycleBeats = 2.f };
			for (const wchar_t* clipName : BeatSyncedLocomotionClipNames)
				GM_ASSERT_RETURN_VAL(animationSync->AddClipSyncRule(clipName, twoBeatSync), false, "Combat Monster 이동 Animation 동기화 규칙 등록에 실패했습니다. clip=%ls", clipName);
		}

		MonsterStateMachineComponent* stateMachine = nullptr;
		switch (data.type)
		{
		case MonsterType::Sjango:
			rigidbody->SetKinematic(true);
			stateMachine = monster.AddComponent<SjangoStateMachineComponent>();
			break;

		case MonsterType::Sword:
		{
			BoxCollider3DComponent* attackCollider = monster.AddComponent<BoxCollider3DComponent>();
			GM_ASSERT_RETURN_VAL(attackCollider, false, "Sword Attack Collider 생성에 실패했습니다.");
			attackCollider->SetColliderId(L"Attack");
			attackCollider->SetLocalCenter(Vector3{ 0.f, 0.75f, 1.2f });
			attackCollider->SetSize(Vector3{ 2.f, 1.5f, 3.2f });
			attackCollider->SetCollisionLayer(HiFiRushCollisionLayer::MonsterAttack);
			attackCollider->SetCollisionMask(HiFiRushCollisionLayer::Player);

			HitBoxComponent* hitBox = monster.AddComponent<HitBoxComponent>(*attackCollider);
			GM_ASSERT_RETURN_VAL(hitBox, false, "Sword HitBoxComponent 생성에 실패했습니다.");
			hitBox->SetDamage(data.attackDamage);
			hitBox->SetHitReactionType(HitReactionType::WeakKnockback);

			SocketComponent* socketComponent = monster.AddComponent<SocketComponent>();
			GM_ASSERT_RETURN_VAL(socketComponent, false, "Sword SocketComponent 생성에 실패했습니다.");
			Socket weaponSocket{};
			weaponSocket.boneName = L"r_hand_attach_00";
			weaponSocket.rotation = Quaternion::CreateFromAxisAngle(Vector3{ 0.f, 1.f, 0.f }, Math::GM_PI);
			socketComponent->AddSocket(L"Sword.Weapon", weaponSocket);

			stateMachine = monster.AddComponent<SwordStateMachineComponent>(data.attackRangeMin, data.attackRangeMax);
			break;
		}

		case MonsterType::Gunner:
		{
			stateMachine = monster.AddComponent<GunnerStateMachineComponent>(data.attackRangeMin, data.attackRangeMax, data.attackDamage);
			break;
		}

		default:
			return false;
		}

		GM_ASSERT_RETURN_VAL(stateMachine, false, "MonsterStateMachineComponent 생성에 실패했습니다.");
		navMeshController->SetUseGroundCollision(true);
		return true;
	}

	bool MonsterSpawner::AddActivationComponents(GameObject& monster, const MonsterSpawnData& data) const
	{
		if (data.activationTriggerId.empty())
			return data.playAppearanceAnimation == false;

		if (data.playAppearanceAnimation)
		{
			SkeletalAnimatorComponent* animator = monster.GetComponent<SkeletalAnimatorComponent>();
			GM_ASSERT_RETURN_VAL(animator, false, "Trigger Monster에 SkeletalAnimatorComponent가 없습니다.");

			const std::wstring appearanceClipName = GetAppearanceAnimationClipName(data.type);
			GM_ASSERT_RETURN_VAL(appearanceClipName.empty() == false, false, "Appearance Animation을 지원하지 않는 Monster Type입니다.");

			BeatTriggeredSkeletalAnimationDesc appearanceDesc{};
			appearanceDesc.triggerId = data.activationTriggerId;
			appearanceDesc.beatOffset = data.appearanceBeatOffset;
			appearanceDesc.clipName = appearanceClipName;
			appearanceDesc.initiallyVisible = false;
			appearanceDesc.hideWhenCompleted = false;
			GM_ASSERT_RETURN_VAL(monster.AddComponent<BeatTriggeredSkeletalAnimationComponent>(HiFiRushStatics::GetBeatSystem(), *animator, std::move(appearanceDesc)), false, "Monster Appearance Animation Component 생성에 실패했습니다.");
		}

		MonsterCombatActivationDesc combatActivationDesc{};
		combatActivationDesc.triggerId = data.activationTriggerId;
		combatActivationDesc.beatOffset = data.combatBeatOffset;
		GM_ASSERT_RETURN_VAL(monster.AddComponent<MonsterCombatActivationComponent>(HiFiRushStatics::GetBeatSystem(), std::move(combatActivationDesc)), false, "Monster Combat Activation Component 생성에 실패했습니다.");
		return true;
	}
}
