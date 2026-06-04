#include "MainScene.h"
#include "MainHUDWidget.h"
#include "ChiStateMachineComponent.h"
#include "ChiMoveComponent.h"
#include "Application.h"
#include "Resources.h"
#include "GameObject.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "CameraComponent.h"
#include "PhysicsSystem.h"
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

namespace gm
{
	void MainScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics2D);
		APPLICATION.GetUIManager().ClearViewportWidgets();
	}

	void MainScene::OnInitialize()
	{
		InitializeSubObject();
		InitializeStaticMeshTest();
		InitializePlayer();
		GetCameraManager()->SetPixelSnapEnabled(false);
	}

	void MainScene::InitializePlayer()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* player = SpawnGameObject<GameObject>({ 0.f, 0.f, 0.f });
		TransformComponent* transform = player->GetTransform();
		transform->SetScale(Vector3{ 1.f, 1.f, 1.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalMeshComponent = player->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		player->AddComponent<SkeletalAnimatorComponent>();

		ChiMoveComponent* moveComponent = player->AddComponent<ChiMoveComponent>();
		//moveComponent->SetRotationYawOffset(Math::GM_PI);

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket socket{};
		socket.position = Vector3{ 0.f, 1.2f, 0.f };
		socketComponent->AddSocket(L"Player.Camera", socket);

		player->AddComponent<ChiStateMachineComponent>();

		WidgetComponent* userWidget = player->AddComponent<WidgetComponent>();
		userWidget->SetUserWidget<MainHUDWidget>();
		userWidget->SetScreenOffset(Vector2{ 0.f, -300.f });

		InitializeCamera(player);
	}

	void MainScene::InitializeSubObject()
	{
		// BackGround
		auto BackGround = SpawnGameObject<GameObject>({ 0.f, 0.f, 500.f });
		auto spriteComponent = BackGround->AddComponent<SpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteComponent->SetTexture(texture);
		auto transform = BackGround->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });

		// Monster
		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ 200.f * i, 300.f, 0.f});
			auto spriteComponent = monster->AddComponent<SpriteComponent>();
			auto texture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			spriteComponent->SetTexture(texture);

			auto transform = monster->GetTransform();
			transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0.f, -100.f, 0.f });
		BoxCollider2DComponent* groundCollider = ground->AddComponent<BoxCollider2DComponent>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void MainScene::InitializeStaticMeshTest()
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

	void MainScene::InitializeSkeletalMeshTest()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi skeletalMesh가 로드되지 않았습니다.");

		GameObject* testObject = SpawnGameObject<GameObject>({ -350.f, -200.f, 200.f });
		TransformComponent* transform = testObject->GetTransform();
		transform->SetScale(Vector3{ 250.f, 250.f, 250.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalmeshComponent = testObject->AddComponent<SkeletalMeshComponent>();
		skeletalmeshComponent->SetSkeletalMesh(skeletalMesh);
		SkeletalAnimatorComponent* animatorComponent = testObject->AddComponent<SkeletalAnimatorComponent>();

		std::shared_ptr<SkeletalAnimationClip> animationClip = APPLICATION.GetResources().Find<SkeletalAnimationClip>(L"chi.DefaultAnimation");
		GM_ASSERT_RETURN(animationClip, "chi.DefaultAnimation SkeletalAnimationClip이 로드되지 않았습니다.");
		animatorComponent->AddClip(L"Default", animationClip);
		animatorComponent->Play(L"Default");
		//animatorComponent->SetPlayRate(0.f);
	}

	void MainScene::InitializeCamera(GameObject* player)
	{
		auto cameraObject = SpawnGameObject<GameObject>({ 0.f, 0.f, 0.f });

		CameraFollowComponent* followComponent = cameraObject->AddComponent<CameraFollowComponent>();
		followComponent->SetTarget(*player, L"Player.Camera");
		followComponent->SetDistance(3.5f);
		followComponent->SetPitch(Math::DegreesToRadians(15.f));

		auto cameraComponent = cameraObject->AddComponent<CameraComponent>();
		const float aspectRatio = static_cast<float>(APPLICATION.GetWidth()) / static_cast<float>(APPLICATION.GetHeight());
		cameraComponent->SetPerspective(Math::GM_PI / 3.f, aspectRatio, 0.1f, 5000.f);
		GetCameraManager()->RegisterCamera(L"PlayerCamera", cameraComponent);
	}
}
