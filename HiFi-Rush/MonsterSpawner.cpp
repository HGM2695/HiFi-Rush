#include "MonsterSpawner.h"

#include "BoxCollider3DComponent.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "HurtBoxComponent.h"
#include "MathUtil.h"
#include "MonsterCombatComponent.h"
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
#include "TransformComponent.h"

namespace gm
{
	MonsterSpawner::MonsterSpawner(Resources& resources)
		: _resources(resources)
	{
	}

	bool MonsterSpawner::Spawn(Scene& scene, const std::vector<MonsterSpawnData>& spawnDataList) const
	{
		for (const MonsterSpawnData& spawnData : spawnDataList)
		{
			GM_ASSERT_RETURN_VAL(Spawn(scene, spawnData), false, "Monster 생성에 실패했습니다.");
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

		Rigidbody3DComponent* rigidbody = monster.AddComponent<Rigidbody3DComponent>();
		GM_ASSERT_RETURN_VAL(rigidbody, false, "Monster Rigidbody3DComponent 생성에 실패했습니다.");
		rigidbody->SetGravityScale(3.f);

		BoxCollider3DComponent* bodyCollider = monster.AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(bodyCollider, false, "Monster Body Collider 생성에 실패했습니다.");
		bodyCollider->SetColliderId(L"Body");
		bodyCollider->SetLocalCenter(data.bodyColliderCenter);
		bodyCollider->SetSize(data.bodyColliderSize);
		bodyCollider->SetCollisionLayer(HiFiRushCollisionLayer::Monster);
		bodyCollider->SetCollisionMask(AllCollisionLayers & ~(HiFiRushCollisionLayer::PlayerAttack | HiFiRushCollisionLayer::MonsterAttack));

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
		navMeshController->SetGroundCollisionEnabled(true);

		GM_ASSERT_RETURN_VAL(monster.AddComponent<CharacterMovementComponent>(data.moveSpeed, data.rotationInterpSpeed), false, "Monster CharacterMovementComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(monster.AddComponent<MonsterCombatComponent>(data.type, HiFiRushStatics::GetBeatSystem(), data.attackCooldownBeats), false, "MonsterCombatComponent 생성에 실패했습니다.");
		MonsterStateMachineComponent* stateMachine = nullptr;
		switch (data.type)
		{
		case MonsterType::Sjango:
			rigidbody->SetKinematic(true);
			stateMachine = monster.AddComponent<SjangoStateMachineComponent>();
			break;

		case MonsterType::Sword:
		{
			SocketComponent* socketComponent = monster.AddComponent<SocketComponent>();
			GM_ASSERT_RETURN_VAL(socketComponent, false, "Sword SocketComponent 생성에 실패했습니다.");
			Socket weaponSocket{};
			weaponSocket.boneName = L"r_hand_attach_00";
			weaponSocket.rotation = Quaternion::CreateFromAxisAngle(Vector3{ 0.f, 1.f, 0.f }, Math::GM_PI);
			socketComponent->AddSocket(L"Sword.Weapon", weaponSocket);

			stateMachine = monster.AddComponent<MonsterStateMachineComponent>();
			break;
		}

		case MonsterType::Gunner:
			stateMachine = monster.AddComponent<MonsterStateMachineComponent>();
			break;

		default:
			return false;
		}

		GM_ASSERT_RETURN_VAL(stateMachine, false, "MonsterStateMachineComponent 생성에 실패했습니다.");
		return true;
	}
}
