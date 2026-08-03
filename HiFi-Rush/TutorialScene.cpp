#include "TutorialScene.h"
#include "MainHUDWidget.h"
#include "ChiStateMachineComponent.h"
#include "ChiMoveComponent.h"
#include "Application.h"
#include "Input.h"
#include "Resources.h"
#include "GameObject.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "CameraComponent.h"
#include "PhysicsSystem.h"
#include "Rigidbody3DComponent.h"
#include "NavMeshControllerComponent.h"
#include "NavMeshSystem.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "BoxCollider2DComponent.h"
#include "WidgetComponent.h"
#include "CameraManager.h"
#include "TransformComponent.h"
#include "SocketComponent.h"
#include "CameraFollowComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalAnimationClip.h"
#include "NavigationMesh.h"
#include "BinaryEnvironmentMapLoader.h"
#include "EnvironmentMapTypes.h"
#include "EnvironmentSpawner.h"
#include "Paths.h"
#include "SceneDebugTools.h"

namespace gm
{
	void TutorialScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);
		APPLICATION.GetUIManager().ClearViewportWidgets();
		APPLICATION.GetInput().SetCursorLocked(true);

		std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"tutorial");
		GM_ASSERT_RETURN(navigationMesh, "tutorial NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);
		GetCameraManager()->SetActiveCamera(L"PlayerCamera");
	}

	void TutorialScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void TutorialScene::OnInitialize()
	{
		//InitializeSubObject();
		InitializeEnvironment();
		InitializePlayer();
		GetCameraManager()->SetPixelSnapEnabled(false);
	}

	void TutorialScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
	}

	void TutorialScene::InitializeEnvironment()
	{
		EnvironmentMapData mapData{};
		GM_ASSERT_RETURN(BinaryEnvironmentMapLoader::Load(GetMapPath(L"TutorialEnvironmentMap.bin"), mapData), "Tutorial 환경 맵을 로드하지 못했습니다.");

		EnvironmentSpawner spawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN(spawner.Spawn(*this, mapData), "Tutorial 환경 오브젝트 생성에 실패했습니다.");
	}

	void TutorialScene::InitializePlayer()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* player = SpawnGameObject<GameObject>(Vector3{ 3.f, 0.f, 0.f });
		TransformComponent* transform = player->GetTransform();
		transform->SetScale(Vector3{ 1.f, 1.f, 1.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalMeshComponent = player->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		player->AddComponent<SkeletalAnimatorComponent>();

		ChiMoveComponent* moveComponent = player->AddComponent<ChiMoveComponent>();
		Rigidbody3DComponent* rigidbody = player->AddComponent<Rigidbody3DComponent>();
		rigidbody->SetGravityScale(3.f);
		NavMeshControllerComponent* navMeshController = player->AddComponent<NavMeshControllerComponent>();
		navMeshController->SetGroundCollisionEnabled(true);
		//moveComponent->SetRotationYawOffset(Math::GM_PI);

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket socket{};
		socket.position = Vector3{ 0.f, 1.2f, 0.f };
		socketComponent->AddSocket(L"Player.Camera", socket);

		player->AddComponent<ChiStateMachineComponent>();

		CameraComponent* playerCamera = InitializeCamera(player);
		moveComponent->SetMovementCamera(*playerCamera);
	}

	void TutorialScene::InitializeSubObject()
	{
		auto background = SpawnGameObject<GameObject>({ 0.f, 0.f, 500.f });
		auto spriteComponent = background->AddComponent<SpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteComponent->SetTexture(texture);
		auto transform = background->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()) });

		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ 200.f * i, 300.f, 0.f });
			auto monsterSprite = monster->AddComponent<SpriteComponent>();
			auto monsterTexture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			monsterSprite->SetTexture(monsterTexture);

			auto monsterTransform = monster->GetTransform();
			monsterTransform->SetScale(Vector2{ static_cast<float>(monsterTexture->GetWidth()), static_cast<float>(monsterTexture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0.f, -100.f, 0.f });
		BoxCollider2DComponent* groundCollider = ground->AddComponent<BoxCollider2DComponent>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void TutorialScene::InitializeStaticMeshTest()
	{
		std::shared_ptr<StaticMesh> staticMesh = APPLICATION.GetResources().Find<StaticMesh>(L"Environment97");
		GM_ASSERT_RETURN(staticMesh, "Environment97 StaticMesh가 로드되지 않았습니다.");

		GameObject* testObject = SpawnGameObject<GameObject>({ 350.f, 0.f, 100.f });
		TransformComponent* transform = testObject->GetTransform();
		transform->SetPosition(Vector3{ 350.f, -200.f, 10.f });
		transform->SetScale(Vector3{ 1000.f, 1000.f, 1000.f });

		StaticMeshComponent* staticMeshComponent = testObject->AddComponent<StaticMeshComponent>();
		staticMeshComponent->SetStaticMesh(staticMesh);
	}

	void TutorialScene::InitializeSkeletalMeshTest()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* testObject = SpawnGameObject<GameObject>({ -350.f, -200.f, 200.f });
		TransformComponent* transform = testObject->GetTransform();
		transform->SetScale(Vector3{ 250.f, 250.f, 250.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalMeshComponent = testObject->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		SkeletalAnimatorComponent* animatorComponent = testObject->AddComponent<SkeletalAnimatorComponent>();

		std::shared_ptr<SkeletalAnimationClip> animationClip = APPLICATION.GetResources().Find<SkeletalAnimationClip>(L"chi.DefaultAnimation");
		GM_ASSERT_RETURN(animationClip, "chi.DefaultAnimation SkeletalAnimationClip이 로드되지 않았습니다.");
		animatorComponent->AddClip(L"Default", animationClip);
		animatorComponent->Play(L"Default");
	}

	CameraComponent* TutorialScene::InitializeCamera(GameObject* player)
	{
		auto cameraObject = SpawnGameObject<GameObject>({ 0.f, 0.f, 0.f });

		CameraFollowComponent* followComponent = cameraObject->AddComponent<CameraFollowComponent>();
		followComponent->SetTarget(*player, L"Player.Camera");
		followComponent->SetDistance(3.5f);
		followComponent->SetPitch(Math::DegreesToRadians(15.f));
		followComponent->SetBottomDistanceLimit(-0.9f);

		auto cameraComponent = cameraObject->AddComponent<CameraComponent>();
		const float aspectRatio = static_cast<float>(APPLICATION.GetWidth()) / static_cast<float>(APPLICATION.GetHeight());
		cameraComponent->SetPerspective(Math::GM_PI / 3.f, aspectRatio, 0.1f, 5000.f);
		GetCameraManager()->RegisterCamera(L"PlayerCamera", cameraComponent);
		return cameraComponent;
	}
}
