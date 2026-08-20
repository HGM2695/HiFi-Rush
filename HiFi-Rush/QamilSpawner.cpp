#include "QamilSpawner.h"

#include "BoxCollider3DComponent.h"
#include "CombatTargetComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "HurtBoxComponent.h"
#include "QamilAnimationTypes.h"
#include "QamilPhaseTriggerComponent.h"
#include "QamilResources.h"
#include "QamilStateMachineComponent.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SocketColliderComponent.h"
#include "SocketComponent.h"
#include "SphereCollider3DComponent.h"
#include "TransformComponent.h"
#include "TriggeredMaterialOverrideComponent.h"

#include <algorithm>

namespace gm
{
	QamilSpawner::QamilSpawner(Resources& resources)
		: _resources(resources)
	{
	}

	GameObject* QamilSpawner::Spawn(Scene& scene, const QamilSpawnDesc& desc) const
	{
		GM_ASSERT_RETURN_VAL(desc.maxHealth > 0, nullptr, "Qamil Max Health는 0보다 커야 합니다.");

		const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(QamilSkeletalMeshResourceKey);
		GM_ASSERT_RETURN_VAL(skeletalMesh, nullptr, "Qamil SkeletalMesh가 로드되지 않았습니다. key=%ls", QamilSkeletalMeshResourceKey);

		GameObject* qamil = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(qamil, nullptr, "Qamil GameObject 생성에 실패했습니다.");
		qamil->GetTransform()->SetWorldMatrix(desc.world);

		SkeletalMeshComponent* meshComponent = qamil->AddComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL(meshComponent, nullptr, "Qamil SkeletalMeshComponent 생성에 실패했습니다.");
		meshComponent->SetSkeletalMesh(skeletalMesh);

		TriggeredMaterialOverrideDesc phase3MaterialDesc{};
		phase3MaterialDesc.triggerId = QamilPhase3TriggerId;
		for (uint32 sectionIndex : QamilPhase3BodySectionIndices)
		{
			const MeshSection* section = skeletalMesh->GetSection(sectionIndex);
			GM_ASSERT_RETURN_VAL(section, nullptr, "Qamil Phase 3 Material Override 대상 Section이 없습니다. section=%u", sectionIndex);
			const bool hasMaterialSlot = std::any_of(phase3MaterialDesc.overrides.begin(), phase3MaterialDesc.overrides.end(), [section](const MaterialTextureOverrideDesc& materialOverride) { return materialOverride.materialSlot == section->textureSetIndex; });
			if (hasMaterialSlot == false)
				phase3MaterialDesc.overrides.push_back({ section->textureSetIndex, TextureSlot::BaseColor, QamilPhase3BodyTextureResourceKey });
		}
		GM_ASSERT_RETURN_VAL(qamil->AddComponent<TriggeredMaterialOverrideComponent>(_resources, HiFiRushStatics::GetBeatSystem(), std::move(phase3MaterialDesc)), nullptr, "Qamil Phase 3 Material Override 구성에 실패했습니다.");

		SkeletalAnimatorComponent* animator = qamil->AddComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, nullptr, "Qamil SkeletalAnimatorComponent 생성에 실패했습니다.");
		for (uint32 animationIndex = 0; animationIndex < QamilAnimationIdCount; ++animationIndex)
		{
			const QamilAnimationId animationId = static_cast<QamilAnimationId>(animationIndex);
			const std::wstring animationKey = GetQamilAnimationClipKey(animationId);
			const std::shared_ptr<SkeletalAnimationClip> animation = _resources.Find<SkeletalAnimationClip>(animationKey);
			GM_ASSERT_RETURN_VAL(animation, nullptr, "Qamil Animation이 로드되지 않았습니다. key=%ls", animationKey.c_str());
			GM_ASSERT_RETURN_VAL(animator->AddClip(GetQamilAnimationClipName(animationId), animation), nullptr, "Qamil Animation 등록에 실패했습니다. key=%ls", animationKey.c_str());
		}

		Rigidbody3DComponent* rigidbody = qamil->AddComponent<Rigidbody3DComponent>();
		GM_ASSERT_RETURN_VAL(rigidbody, nullptr, "Qamil Rigidbody3DComponent 생성에 실패했습니다.");
		rigidbody->SetKinematic(true);
		rigidbody->SetUseGravity(false);

		GM_ASSERT_RETURN_VAL(qamil->AddComponent<HealthComponent>(desc.maxHealth), nullptr, "Qamil HealthComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(qamil->AddComponent<QamilStateMachineComponent>(), nullptr, "QamilStateMachineComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(qamil->AddComponent<QamilPhaseTriggerComponent>(), nullptr, "QamilPhaseTriggerComponent 생성에 실패했습니다.");
		SocketComponent* sockets = qamil->AddComponent<SocketComponent>();
		GM_ASSERT_RETURN_VAL(sockets, nullptr, "Qamil SocketComponent 생성에 실패했습니다.");
		for (const QamilMissileSocketBinding& binding : QamilMissileSocketBindings)
		{
			Socket launchSocket{};
			launchSocket.boneName = binding.launchBoneName;
			sockets->AddSocket(binding.launchSocketName, launchSocket);
			Socket directionSourceSocket{};
			directionSourceSocket.boneName = binding.directionSourceBoneName;
			sockets->AddSocket(binding.directionSourceSocketName, directionSourceSocket);
		}

		GM_ASSERT_RETURN_VAL(AddSphereHurtBox(*qamil, *sockets, L"Qamil.LeftHand", L"Qamil.LeftHand", L"l_hand", Vector3{ 2.3f, 0.9f, 0.f }, 2.2f, true), nullptr, "Qamil Left Hand HurtBox 구성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(AddSphereHurtBox(*qamil, *sockets, L"Qamil.RightHand", L"Qamil.RightHand", L"r_hand", Vector3{ 2.3f, -0.9f, 0.f }, 2.2f, true), nullptr, "Qamil Right Hand HurtBox 구성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(AddBoxHurtBox(*qamil, *sockets, L"Qamil.Core", L"Qamil.Core", L"core_jnt", Vector3{ 0.f, 2.f, -1.4f }, Vector3{ 6.2f, 5.6f, 5.12f }, false), nullptr, "Qamil Core HurtBox 구성에 실패했습니다.");
		CombatTargetComponent* combatTarget = qamil->AddComponent<CombatTargetComponent>();
		GM_ASSERT_RETURN_VAL(combatTarget, nullptr, "Qamil CombatTargetComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(combatTarget->AddSocketTargetPoint(L"Core", L"Qamil.Core"), nullptr, "Qamil Core Target Point 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(combatTarget->AddSocketTargetPoint(L"LeftHand", L"Qamil.LeftHand"), nullptr, "Qamil Left Hand Target Point 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(combatTarget->AddSocketTargetPoint(L"RightHand", L"Qamil.RightHand"), nullptr, "Qamil Right Hand Target Point 등록에 실패했습니다.");

		return qamil;
	}

	bool QamilSpawner::AddSphereHurtBox(GameObject& qamil, SocketComponent& sockets, const wchar_t* colliderId, const wchar_t* socketName, const wchar_t* boneName, const Vector3& localCenter, float radius, bool isTrigger) const
	{
		Socket socket{};
		socket.boneName = boneName;
		socket.position = localCenter;
		sockets.AddSocket(socketName, socket);

		SphereCollider3DComponent* collider = qamil.AddComponent<SphereCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(collider, false, "Qamil Sphere Hurt Collider 생성에 실패했습니다. collider=%ls", colliderId);
		collider->SetColliderId(colliderId);
		collider->SetRadius(radius);
		collider->SetCollisionLayer(HiFiRushCollisionLayer::Monster);
		collider->SetCollisionMask(HiFiRushCollisionLayer::Player | HiFiRushCollisionLayer::PlayerAttack);
		collider->SetTrigger(isTrigger);
		collider->SetCollisionResponseMode(CollisionResponseMode::Planar);

		GM_ASSERT_RETURN_VAL(qamil.AddComponent<SocketColliderComponent>(*collider, socketName), false, "Qamil Sphere Hurt Collider Socket 연결에 실패했습니다. collider=%ls", colliderId);
		GM_ASSERT_RETURN_VAL(qamil.AddComponent<HurtBoxComponent>(colliderId), false, "Qamil HurtBoxComponent 생성에 실패했습니다. collider=%ls", colliderId);
		return true;
	}

	bool QamilSpawner::AddBoxHurtBox(GameObject& qamil, SocketComponent& sockets, const wchar_t* colliderId, const wchar_t* socketName, const wchar_t* boneName, const Vector3& localCenter, const Vector3& size, bool isTrigger) const
	{
		Socket socket{};
		socket.boneName = boneName;
		socket.position = localCenter;
		sockets.AddSocket(socketName, socket);

		BoxCollider3DComponent* collider = qamil.AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(collider, false, "Qamil Hurt Collider 생성에 실패했습니다. collider=%ls", colliderId);
		collider->SetColliderId(colliderId);
		collider->SetSize(size);
		collider->SetCollisionLayer(HiFiRushCollisionLayer::Monster);
		collider->SetCollisionMask(HiFiRushCollisionLayer::Player | HiFiRushCollisionLayer::PlayerAttack);
		collider->SetTrigger(isTrigger);
		collider->SetCollisionResponseMode(CollisionResponseMode::Planar);

		SocketColliderComponent* socketCollider = qamil.AddComponent<SocketColliderComponent>(*collider, socketName);
		GM_ASSERT_RETURN_VAL(socketCollider, false, "Qamil Hurt Collider Socket 연결에 실패했습니다. collider=%ls", colliderId);
		socketCollider->SetFollowSocketRotation(false);
		GM_ASSERT_RETURN_VAL(qamil.AddComponent<HurtBoxComponent>(colliderId), false, "Qamil HurtBoxComponent 생성에 실패했습니다. collider=%ls", colliderId);
		return true;
	}
}
