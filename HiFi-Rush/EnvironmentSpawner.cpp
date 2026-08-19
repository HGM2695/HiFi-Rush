#include "EnvironmentSpawner.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "GMAssert.h"
#include "GMLog.h"
#include "HiFiRushStatics.h"
#include "MapTypes.h"
#include "Resources.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"

#include <string>

namespace gm
{
	namespace
	{
		std::wstring GetEnvironmentModelKey(uint32 modelIndex)
		{
			return L"Environment" + std::to_wstring(modelIndex);
		}
	}

	EnvironmentSpawner::EnvironmentSpawner(Resources& resources)
		: _resources(resources), _componentFactory(resources, HiFiRushStatics::GetBeatSystem())
	{}

	bool EnvironmentSpawner::Spawn(GameplayScene& scene, const std::vector<EnvironmentObjectData>& objectDatas) const
	{
		for (const EnvironmentObjectData& objectData : objectDatas)
			if (SpawnObject(scene, objectData) == false)
				return false;

		GM_LOG("Environment objects spawned. count=%zu", objectDatas.size());
		return true;
	}

	bool EnvironmentSpawner::SpawnObject(GameplayScene& scene, const EnvironmentObjectData& objectData) const
	{
		GameObject* gameObject = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(gameObject, false, "환경 오브젝트 생성에 실패했습니다.");
		gameObject->GetTransform()->SetWorldMatrix(objectData.world);

		if (objectData.renderType != EnvironmentRenderType::None)
		{
			const std::wstring modelKey = GetEnvironmentModelKey(objectData.modelIndex);
			const std::shared_ptr<StaticMesh> staticMesh = _resources.Find<StaticMesh>(modelKey);
			const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(modelKey);
			GM_ASSERT_RETURN_VAL(staticMesh || skeletalMesh, false, "환경 오브젝트가 참조하는 모델 리소스가 없습니다. key=%ls", modelKey.c_str());

			if (staticMesh)
			{
				StaticMeshComponent* meshComponent = gameObject->AddComponent<StaticMeshComponent>();
				GM_ASSERT_RETURN_VAL(meshComponent, false, "StaticMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
				meshComponent->SetStaticMesh(staticMesh);
			}
			else // skeletalMesh
			{
				SkeletalMeshComponent* meshComponent = gameObject->AddComponent<SkeletalMeshComponent>();
				GM_ASSERT_RETURN_VAL(meshComponent, false, "SkeletalMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
				meshComponent->SetSkeletalMesh(skeletalMesh);

				const std::shared_ptr<SkeletalAnimationClip> defaultClip = _resources.Find<SkeletalAnimationClip>(modelKey + L".DefaultAnimation");
				if (defaultClip)
				{
					SkeletalAnimatorComponent* animator = gameObject->AddComponent<SkeletalAnimatorComponent>();
					GM_ASSERT_RETURN_VAL(animator, false, "SkeletalAnimatorComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
					GM_ASSERT_RETURN_VAL(animator->AddClip(L"Default", defaultClip), false, "환경 오브젝트의 기본 애니메이션 등록에 실패했습니다. key=%ls", modelKey.c_str());
					GM_ASSERT_RETURN_VAL(animator->Play(L"Default"), false, "환경 오브젝트의 기본 애니메이션 재생에 실패했습니다. key=%ls", modelKey.c_str());
				}
			}
		}

		GM_ASSERT_RETURN_VAL(_componentFactory.AddComponents(*gameObject, objectData.components), false, "환경 오브젝트 Component 구성에 실패했습니다.");

		return true;
	}
}
