#include "MainScene.h"
#include "MainHUDWidget.h"
#include "PlayerMovement.h"
#include "PlayerAnimationFSM.h"
#include "Application.h"
#include "Resources.h"
#include "GameObject.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include "Camera.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"
#include "SpriteAnimator.h"
#include "SpriteAnimationClip.h"
#include "UIManager.h"
#include "Rigidbody2D.h"
#include "BoxCollider2D.h"
#include "WidgetComponent.h"
#include "CameraManager.h"
#include "Material.h"
#include "Transform.h"
#include "SocketComponent.h"
#include "SocketFollowComponent.h"

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
		InitializePlayer();
		GetCameraManager()->SetPixelSnapEnabled(true);
	}

	void MainScene::InitializePlayer()
	{
		auto player = SpawnGameObject<GameObject>({ 0, 200 });

		auto transform = player->GetTransform();

		auto spriteRenderer = player->AddComponent<SpriteRenderer>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
		spriteRenderer->SetTexture(texture);
		transform->SetScale(Vector2{ 100.f , 100.f });

		player->AddComponent<PlayerMovement>();

		auto spriteAnimator = player->AddComponent<SpriteAnimator>();
		spriteAnimator->AddClip(L"IdleLeft", L"Player_IdleLeft");
		spriteAnimator->AddClip(L"MoveLeft", L"Player_MoveLeft");
		spriteAnimator->AddClip(L"IdleRight", L"Player_IdleRight");
		spriteAnimator->AddClip(L"MoveRight", L"Player_MoveRight");

		player->AddComponent<PlayerAnimationFSM>();
		Rigidbody2D* rigidbody = player->AddComponent<Rigidbody2D>();
		rigidbody->SetLinearDamping(1.f);

		BoxCollider2D* collider = player->AddComponent<BoxCollider2D>();
		collider->SetSize({ 60.f, 70.f });

		SocketComponent* socketComponent = player->AddComponent<SocketComponent>();
		Socket socket{};
		socket.position = Vector3{ 0.f, 0.f, -1.f };
		socketComponent->AddSocket(L"Player.Camera", socket);

		WidgetComponent* userWidget = player->AddComponent<WidgetComponent>();
		userWidget->SetUserWidget<MainHUDWidget>();

		InitializeCamera(player);
	}

	void MainScene::InitializeSubObject()
	{
		// BackGround
		auto BackGround = SpawnGameObject<GameObject>({ 0, 0 });
		auto spriteRenderer = BackGround->AddComponent<SpriteRenderer>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteRenderer->SetTexture(texture);
		auto transform = BackGround->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });

		// Monster
		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ (float)200 * i, 300 });
			auto spriteRenderer = monster->AddComponent<SpriteRenderer>();
			auto texture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			spriteRenderer->SetTexture(texture);

			auto transform = monster->GetTransform();
			transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0, -250 });
		BoxCollider2D* groundCollider = ground->AddComponent<BoxCollider2D>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void MainScene::InitializeCamera(GameObject* player)
	{
		auto cameraObject = SpawnGameObject<GameObject>({ 0, 0 });

		SocketFollowComponent* followComponent = cameraObject->AddComponent<SocketFollowComponent>();
		followComponent->SetTarget(*player, L"Player.Camera");

		auto cameraComponent = cameraObject->AddComponent<Camera>();
		cameraComponent->SetOrthographic(static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()));
		GetCameraManager()->RegisterCamera(L"PlayerCamera", cameraComponent);
	}
}
