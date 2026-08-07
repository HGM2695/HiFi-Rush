#include "TestScene.h"
#include "BeatSkeletalAnimationSyncComponent.h"
#include "MainHUDWidget.h"
#include "ChiAnimationTypes.h"
#include "ChiStateMachineComponent.h"
#include "ChiMoveComponent.h"
#include "Application.h"
#include "Input.h"
#include "Resources.h"
#include "GameObject.h"
#include "HiFiRushStatics.h"
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
#include "SceneDebugTools.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugInputHandler.h"
#include "IDebugRenderer.h"
#include <algorithm>
#include <cstdio>
#endif

namespace gm
{
#if GM_ENABLE_DEBUG_TOOLS
	namespace
	{
		constexpr const wchar_t* CameraTestDebugType = L"CameraTest";
	}
#endif

	void TestScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);
		APPLICATION.GetUIManager().ClearViewportWidgets();
		APPLICATION.GetInput().SetCursorLocked(true);

		std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"tutorial");
		GM_ASSERT_RETURN(navigationMesh, "tutorial NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);
		GetCameraManager()->SetActiveCamera(L"PlayerCamera");
	}

	void TestScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void TestScene::OnInitialize()
	{
		//InitializeSubObject();
		InitializeStaticMeshTest();
		InitializePlayer();
		GetCameraManager()->SetPixelSnapEnabled(false);

#if GM_ENABLE_DEBUG_TOOLS
		DebugInputHandler::RegisterDebugType(CameraTestDebugType, true);
		DebugInputHandler::Enable(CameraTestDebugType);
#endif
	}

	void TestScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();

#if GM_ENABLE_DEBUG_TOOLS
		TickCameraProjectionDebug(deltaTime);
#endif
	}

	void TestScene::InitializePlayer()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* player = SpawnGameObject<GameObject>(Vector3{ 3.f, 0.f, 0.f });
		TransformComponent* transform = player->GetTransform();
		transform->SetScale(Vector3{ 1.f, 1.f, 1.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalMeshComponent = player->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		SkeletalAnimatorComponent* animator = player->AddComponent<SkeletalAnimatorComponent>();

		ChiMoveComponent* moveComponent = player->AddComponent<ChiMoveComponent>();
		Rigidbody3DComponent* rigidbody = player->AddComponent<Rigidbody3DComponent>();
		rigidbody->SetGravityScale(3.f);
		NavMeshControllerComponent* navMeshController = player->AddComponent<NavMeshControllerComponent>();
		navMeshController->SetGroundCollisionEnabled(true);

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket socket{};
		socket.position = Vector3{ 0.f, 1.2f, 0.f };
		socketComponent->AddSocket(L"Player.Camera", socket);

		player->AddComponent<ChiStateMachineComponent>();

		BeatSkeletalAnimationSyncDesc animationSyncDesc{};
		BeatSkeletalAnimationSyncComponent* animationSync = player->AddComponent<BeatSkeletalAnimationSyncComponent>(HiFiRushStatics::GetBeatSystem(), *animator, animationSyncDesc);
		GM_ASSERT_RETURN(animationSync->AddClipSyncRule(GetChiAnimationName(ChiAnimationId::Idle), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 4.f }), "플레이어 Idle 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");
		GM_ASSERT_RETURN(animationSync->AddClipSyncRule(GetChiAnimationName(ChiAnimationId::RunFront), BeatSkeletalAnimationSyncDesc{ .cycleBeats = 2.f }), "플레이어 Run 애니메이션 비트 동기화 규칙 등록에 실패했습니다.");

		WidgetComponent* userWidget = player->AddComponent<WidgetComponent>();
		userWidget->SetUserWidget<MainHUDWidget>();
		userWidget->SetScreenOffset(Vector2{ 0.f, -300.f });

		CameraComponent* playerCamera = InitializeCamera(player);
		moveComponent->SetMovementCamera(*playerCamera);
	}

	void TestScene::InitializeSubObject()
	{
		auto BackGround = SpawnGameObject<GameObject>({ 0.f, 0.f, 500.f });
		auto spriteComponent = BackGround->AddComponent<SpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteComponent->SetTexture(texture);
		auto transform = BackGround->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()) });

		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ 200.f * i, 300.f, 0.f });
			auto monsterSpriteComponent = monster->AddComponent<SpriteComponent>();
			auto monsterTexture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			monsterSpriteComponent->SetTexture(monsterTexture);

			auto monsterTransform = monster->GetTransform();
			monsterTransform->SetScale(Vector2{ static_cast<float>(monsterTexture->GetWidth()), static_cast<float>(monsterTexture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0.f, -100.f, 0.f });
		BoxCollider2DComponent* groundCollider = ground->AddComponent<BoxCollider2DComponent>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void TestScene::InitializeStaticMeshTest()
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

	void TestScene::InitializeSkeletalMeshTest()
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
	}

	CameraComponent* TestScene::InitializeCamera(GameObject* player)
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

#if GM_ENABLE_DEBUG_TOOLS
	void TestScene::TickCameraProjectionDebug(float deltaTime)
	{
		CameraComponent* activeCamera = GetCameraManager()->GetActiveCamera();
		if (activeCamera == nullptr || activeCamera->GetProjectionMode() != CameraProjectionMode::Perspective)
			return;

		constexpr float minFovY = Math::GM_PI / 12.f;
		constexpr float maxFovY = Math::GM_PI * 2.f / 3.f;
		constexpr float fovSpeed = Math::GM_PI / 3.f;
		constexpr float minAspectRatio = 0.25f;
		constexpr float maxAspectRatio = 4.f;
		constexpr float aspectRatioSpeed = 1.f;

		const float fovAxis =
			(DebugInputHandler::IsTriggered(CameraTestDebugType, KeyCode::F6, KeyState::Repeat) ? 1.f : 0.f) - (DebugInputHandler::IsTriggered(CameraTestDebugType, KeyCode::F5, KeyState::Repeat) ? 1.f : 0.f);
		const float aspectAxis =
			(DebugInputHandler::IsTriggered(CameraTestDebugType, KeyCode::F8, KeyState::Repeat) ? 1.f : 0.f) - (DebugInputHandler::IsTriggered(CameraTestDebugType, KeyCode::F7, KeyState::Repeat) ? 1.f : 0.f);

		if (fovAxis == 0.f && aspectAxis == 0.f)
		{
			wchar_t debugText[128]{};
			swprintf_s(debugText, L"Camera Test\nFOV : %.1f deg\nAspect : %.3f\nF5/F6 : FOV, F7/F8 : Aspect", activeCamera->GetFovYRadians() * 180.f / Math::GM_PI, activeCamera->GetAspectRatio());
			APPLICATION.GetDebugRenderer().RequestDrawText(debugText, Vector2{ 20.f, 60.f }, 16.f, Colors::Green);
			return;
		}

		const float fovY = std::clamp(activeCamera->GetFovYRadians() + fovAxis * fovSpeed * deltaTime, minFovY, maxFovY);
		const float aspectRatio = std::clamp(activeCamera->GetAspectRatio() + aspectAxis * aspectRatioSpeed * deltaTime, minAspectRatio, maxAspectRatio);
		activeCamera->SetPerspective(fovY, aspectRatio, activeCamera->GetNearZ(), activeCamera->GetFarZ());

		wchar_t debugText[128]{};
		swprintf_s(debugText, L"Camera Test\nFOV : %.1f deg\nAspect : %.3f\nF5/F6 : FOV, F7/F8 : Aspect", fovY * 180.f / Math::GM_PI, aspectRatio);
		APPLICATION.GetDebugRenderer().RequestDrawText(debugText, Vector2{ 20.f, 60.f }, 16.f, Colors::Green);
	}
#endif
}
