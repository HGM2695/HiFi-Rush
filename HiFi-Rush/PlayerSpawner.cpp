#include "PlayerSpawner.h"

#include "Application.h"
#include "BeatSkeletalAnimationSyncComponent.h"
#include "BoxCollider3DComponent.h"
#include "CameraComponent.h"
#include "CameraFollowComponent.h"
#include "CameraManager.h"
#include "ChiAnimationTypes.h"
#include "ChiAudioComponent.h"
#include "ChiEffectComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "FreeFlyMoveComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushStatics.h"
#include "HitBoxComponent.h"
#include "HurtBoxComponent.h"
#include "MathUtil.h"
#include "NavMeshControllerComponent.h"
#include "PlayerRuntimeState.h"
#include "PlayerRuntimeStateSyncComponent.h"
#include "PlayerControlComponent.h"
#include "PlayerResources.h"
#include "PlayerTargetingComponent.h"
#include "PhysicsSystem.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "ReverbComponent.h"
#include "RhythmRankComponent.h"
#include "Scene.h"
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
	PlayerSpawner::PlayerSpawner(Resources& resources)
		: _resources(resources)
	{
	}

	GameObject* PlayerSpawner::Spawn(Scene& scene, const PlayerSpawnDesc& desc, PlayerRuntimeState& runtimeState) const
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(ChiSkeletalMeshResourceKey);
		GM_ASSERT_RETURN_VAL(skeletalMesh, nullptr, "chi SkeletalMesh가 로드되지 않았습니다.");
		std::shared_ptr<StaticMesh> guitarMesh = _resources.Find<StaticMesh>(ChiGuitarResourceKey);
		GM_ASSERT_RETURN_VAL(guitarMesh, nullptr, "Chi Guitar StaticMesh가 로드되지 않았습니다.");

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
		bodyCollider->SetCollisionMask(AllCollisionLayers & ~(HiFiRushCollisionLayer::PlayerAttack | HiFiRushCollisionLayer::MonsterAttack));
		bodyCollider->SetCollisionResponseMode(CollisionResponseMode::Planar);

		BoxCollider3DComponent* hurtCollider = player->AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(hurtCollider, nullptr, "Player Hurt Collider 생성에 실패했습니다.");
		hurtCollider->SetColliderId(L"HurtBox");
		hurtCollider->SetLocalCenter(Vector3{ 0.f, 0.9f, 0.f });
		hurtCollider->SetSize(Vector3{ 0.8f, 1.8f, 0.8f });
		hurtCollider->SetCollisionLayer(HiFiRushCollisionLayer::Player);
		hurtCollider->SetCollisionMask(HiFiRushCollisionLayer::MonsterAttack);
		hurtCollider->SetTrigger(true);

		HealthComponent* healthComponent = player->AddComponent<HealthComponent>(runtimeState.maxHealth);
		GM_ASSERT_RETURN_VAL(healthComponent, nullptr, "Player HealthComponent 생성에 실패했습니다.");
		ReverbComponent* reverbComponent = player->AddComponent<ReverbComponent>(runtimeState.maxReverb);
		GM_ASSERT_RETURN_VAL(reverbComponent, nullptr, "Player ReverbComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(player->AddComponent<PlayerRuntimeStateSyncComponent>(runtimeState), nullptr, "PlayerRuntimeStateSyncComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(player->AddComponent<HurtBoxComponent>(L"HurtBox"), nullptr, "Player HurtBoxComponent 생성에 실패했습니다.");

		NavMeshControllerComponent* navMeshController = player->AddComponent<NavMeshControllerComponent>();
		navMeshController->SetUseGroundCollision(true);

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket cameraSocket{};
		cameraSocket.position = Vector3{ 0.f, 1.2f, 0.f };
		socketComponent->AddSocket(L"Player.Camera", cameraSocket);
		Socket weaponSocket{};
		weaponSocket.boneName = L"r_attach_hand_00";
		weaponSocket.rotation = Quaternion::CreateFromYawPitchRoll(Math::GM_PI * 0.5f, Math::GM_PI, -Math::GM_PI * 0.5f);
		socketComponent->AddSocket(L"Player.Weapon", weaponSocket);
		for (const ChiEffectSocketBinding& binding : ChiEffectSocketBindings)
		{
			Socket effectSocket{};
			effectSocket.boneName = binding.boneName;
			socketComponent->AddSocket(binding.socketName, effectSocket);
		}

		GameObject* weapon = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(weapon, nullptr, "Player Weapon GameObject 생성에 실패했습니다.");
		StaticMeshComponent* weaponMeshComponent = weapon->AddComponent<StaticMeshComponent>();
		GM_ASSERT_RETURN_VAL(weaponMeshComponent, nullptr, "Player Weapon StaticMeshComponent 생성에 실패했습니다.");
		weaponMeshComponent->SetStaticMesh(guitarMesh);
		BoxCollider3DComponent* weaponCollider = weapon->AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN_VAL(weaponCollider, nullptr, "Player Weapon Collider 생성에 실패했습니다.");
		weaponCollider->SetColliderId(L"WeaponAttack");
		weaponCollider->SetLocalCenter(Vector3{ 0.f, 0.5f, 0.f });
		weaponCollider->SetSize(Vector3{ 0.8f, 1.5f, 0.8f });
		weaponCollider->SetCollisionLayer(HiFiRushCollisionLayer::PlayerAttack);
		weaponCollider->SetCollisionMask(HiFiRushCollisionLayer::Monster);
		HitBoxComponent* weaponHitBox = weapon->AddComponent<HitBoxComponent>(*weaponCollider);
		GM_ASSERT_RETURN_VAL(weaponHitBox, nullptr, "Player Weapon HitBoxComponent 생성에 실패했습니다.");
		SocketFollowComponent* weaponFollowComponent = weapon->AddComponent<SocketFollowComponent>();
		GM_ASSERT_RETURN_VAL(weaponFollowComponent, nullptr, "Player Weapon SocketFollowComponent 생성에 실패했습니다.");
		weaponFollowComponent->SetTarget(*player, L"Player.Weapon");
		weaponFollowComponent->SetDestroyWithTarget(true);

		GM_ASSERT_RETURN_VAL(player->AddComponent<ChiEffectComponent>(_resources, HiFiRushStatics::GetEffectPresets()), nullptr, "Player ChiEffectComponent 생성에 실패했습니다.");
		ChiStateMachineComponent* stateMachineComponent = player->AddComponent<ChiStateMachineComponent>(weaponHitBox);
		GM_ASSERT_RETURN_VAL(stateMachineComponent, nullptr, "Player ChiStateMachineComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(player->AddComponent<ChiAudioComponent>(), nullptr, "Player ChiAudioComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(player->AddComponent<RhythmRankComponent>(), nullptr, "Player RhythmRankComponent 생성에 실패했습니다.");

		BeatSkeletalAnimationSyncDesc animationSyncDesc{};
		BeatSkeletalAnimationSyncComponent* animationSync = player->AddComponent<BeatSkeletalAnimationSyncComponent>(HiFiRushStatics::GetBeatSystem(), *animator, animationSyncDesc);
		GM_ASSERT_RETURN_VAL(animationSync->AddClipSyncRule(GetChiAnimationClipName(ChiAnimationClipId::Idle), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 4.f }), nullptr, "플레이어 Idle 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(animationSync->AddClipSyncRule(GetChiAnimationClipName(ChiAnimationClipId::RunFront), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 2.f }), nullptr, "플레이어 Run 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");

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
		cameraComponent->SetPerspective(Math::GM_PI / 3.f, aspectRatio, 0.1f, 500.f);
		scene.GetCameraManager()->RegisterCamera(PlayerCameraKey, cameraComponent);
		GM_ASSERT_RETURN_VAL(player->AddComponent<PlayerTargetingComponent>(APPLICATION.GetPhysicsSystem().GetPhysicsSystem3D(), *cameraComponent), nullptr, "PlayerTargetingComponent 생성에 실패했습니다.");
		moveComponent->SetMovementCamera(*cameraComponent);
		GM_ASSERT_RETURN_VAL(player->AddComponent<PlayerControlComponent>(*moveComponent, *stateMachineComponent, *followComponent), nullptr, "PlayerControlComponent 생성에 실패했습니다.");

#if GM_ENABLE_DEBUG_TOOLS
		FreeFlyMoveComponent* freeFlyMoveComponent = player->AddComponent<FreeFlyMoveComponent>();
		freeFlyMoveComponent->SetMovementCamera(*cameraComponent);
#endif

		return player;
	}
}
