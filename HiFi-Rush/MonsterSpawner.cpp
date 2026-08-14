#include "MonsterSpawner.h"

#include "BoxCollider3DComponent.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "HurtBoxComponent.h"
#include "MonsterCombatComponent.h"
#include "MonsterResources.h"
#include "MonsterStateMachineComponent.h"
#include "MonsterTypes.h"
#include "NavMeshControllerComponent.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
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
		const MonsterResourceInfo* resourceInfo = FindMonsterResourceInfo(data.type);
		GM_ASSERT_RETURN_VAL(resourceInfo, nullptr, "지원하지 않는 Monster Type입니다.");

		const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(resourceInfo->resourceKey);
		GM_ASSERT_RETURN_VAL(skeletalMesh, nullptr, "Monster SkeletalMesh가 로드되지 않았습니다. key=%ls", resourceInfo->resourceKey);

		const std::wstring defaultAnimationKey = GetMonsterDefaultAnimationResourceKey(data.type);
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
		GM_ASSERT_RETURN_VAL(monster.AddComponent<MonsterStateMachineComponent>(), false, "MonsterStateMachineComponent 생성에 실패했습니다.");
		return true;
	}
}
