#include "PlayerSpawner.h"

#include "Application.h"
#include "BeatSkeletalAnimationSyncComponent.h"
#include "BoxCollider3DComponent.h"
#include "CameraComponent.h"
#include "CameraFollowComponent.h"
#include "CameraManager.h"
#include "ChiAnimationTypes.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "FreeFlyMoveComponent.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "MathUtil.h"
#include "NavMeshControllerComponent.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

namespace gm
{
	PlayerSpawner::PlayerSpawner(Resources& resources)
		: _resources(resources)
	{
	}

	GameObject* PlayerSpawner::Spawn(Scene& scene, const PlayerSpawnDesc& desc) const
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN_VAL(skeletalMesh, nullptr, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* player = scene.SpawnGameObject<GameObject>(desc.position);
		player->GetTransform()->SetRotationY(desc.rotationY);

		SkeletalMeshComponent* skeletalMeshComponent = player->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		SkeletalAnimatorComponent* animator = player->AddComponent<SkeletalAnimatorComponent>();

		ChiMoveComponent* moveComponent = player->AddComponent<ChiMoveComponent>();
		Rigidbody3DComponent* rigidbody = player->AddComponent<Rigidbody3DComponent>();
		rigidbody->SetGravityScale(3.f);

		BoxCollider3DComponent* bodyCollider = player->AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(bodyCollider, nullptr, "Player Body Collider 생성에 실패했습니다.");
		bodyCollider->SetColliderId(L"Body");
		bodyCollider->SetLocalCenter(Vector3{ 0.f, 0.9f, 0.f });
		bodyCollider->SetSize(Vector3{ 0.8f, 1.8f, 0.8f });
		bodyCollider->SetCollisionLayer(HiFiRushCollisionLayer::Player);

		NavMeshControllerComponent* navMeshController = player->AddComponent<NavMeshControllerComponent>();
		navMeshController->SetGroundCollisionEnabled(true);

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket cameraSocket{};
		cameraSocket.position = Vector3{ 0.f, 1.2f, 0.f };
		socketComponent->AddSocket(L"Player.Camera", cameraSocket);

		player->AddComponent<ChiStateMachineComponent>();

		BeatSkeletalAnimationSyncDesc animationSyncDesc{};
		BeatSkeletalAnimationSyncComponent* animationSync = player->AddComponent<BeatSkeletalAnimationSyncComponent>(HiFiRushStatics::GetBeatSystem(), *animator, animationSyncDesc);
		GM_ASSERT_RETURN_VAL(animationSync->AddClipSyncRule(GetChiAnimationName(ChiAnimationId::Idle), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 4.f }), nullptr, "플레이어 Idle 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(animationSync->AddClipSyncRule(GetChiAnimationName(ChiAnimationId::RunFront), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 2.f }), nullptr, "플레이어 Run 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");

		GameObject* cameraObject = scene.SpawnGameObject<GameObject>();
		CameraFollowComponent* followComponent = cameraObject->AddComponent<CameraFollowComponent>();
		followComponent->SetTarget(*player, L"Player.Camera");
		followComponent->SetDistance(desc.cameraDistance);
		followComponent->SetYaw(desc.cameraYaw);
		followComponent->SetPitch(desc.cameraPitch);
		followComponent->SetHeight(desc.cameraHeight);
		followComponent->SetBottomDistanceLimit(-0.9f);

		CameraComponent* cameraComponent = cameraObject->AddComponent<CameraComponent>();
		const float aspectRatio = static_cast<float>(APPLICATION.GetWidth()) / static_cast<float>(APPLICATION.GetHeight());
		cameraComponent->SetPerspective(Math::GM_PI / 3.f, aspectRatio, 0.1f, 5000.f);
		scene.GetCameraManager()->RegisterCamera(PlayerCameraKey, cameraComponent);
		moveComponent->SetMovementCamera(*cameraComponent);

#if GM_ENABLE_DEBUG_TOOLS
		FreeFlyMoveComponent* freeFlyMoveComponent = player->AddComponent<FreeFlyMoveComponent>();
		freeFlyMoveComponent->SetMovementCamera(*cameraComponent);
#endif

		return player;
	}
}
