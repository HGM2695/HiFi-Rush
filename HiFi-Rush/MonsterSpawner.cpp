#include "MonsterSpawner.h"

#include "GameObject.h"
#include "MonsterResources.h"
#include "MonsterTypes.h"
#include "Resources.h"
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

		return monster;
	}
}
