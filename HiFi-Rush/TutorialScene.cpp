#include "TutorialScene.h"
#include "Application.h"
#include "Input.h"
#include "Resources.h"
#include "GameObject.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "PhysicsSystem.h"
#include "NavMeshSystem.h"
#include "UIManager.h"
#include "BoxCollider2DComponent.h"
#include "CameraManager.h"
#include "TransformComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalAnimationClip.h"
#include "NavigationMesh.h"
#include "HiFiRushAudio.h"
#include "PlayerSpawner.h"
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
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Tutorial);
	}

	void TutorialScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void TutorialScene::OnInitialize()
	{
		//InitializeSubObject();
		GM_ASSERT_RETURN(InitializeMap(L"TutorialMap"), "Tutorial Map 구성에 실패했습니다.");

		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ 3.f, 0.f, 0.f };
		playerDesc.rotationY = Math::GM_PI;
		playerDesc.cameraDistance = 3.5f;
		playerDesc.cameraPitch = Math::DegreesToRadians(15.f);
		GM_ASSERT_RETURN(InitializePlayer(playerDesc), "Tutorial Player 생성에 실패했습니다.");

		GetCameraManager()->SetPixelSnapEnabled(false);
	}

	void TutorialScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
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
}
