#include "MainScene.h"
#include "MainHUDWidget.h"
#include "PlayerMovementComponent.h"
#include "PlayerAnimationFSMComponent.h"
#include "Application.h"
#include "Resources.h"
#include "GameObject.h"
#include "AnimatedSpriteComponent.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "CameraComponent.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"
#include "SpriteAnimationClip.h"
#include "UIManager.h"
#include "Rigidbody2DComponent.h"
#include "BoxCollider2DComponent.h"
#include "WidgetComponent.h"
#include "CameraManager.h"
#include "Material.h"
#include "TransformComponent.h"
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

		auto spriteComponent = player->AddComponent<AnimatedSpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
		spriteComponent->SetTexture(texture);
		transform->SetScale(Vector2{ 100.f , 100.f });

		player->AddComponent<PlayerMovementComponent>();

		SpriteAnimator& animator = spriteComponent->GetAnimator();
		animator.AddClip(L"IdleLeft", L"Player_IdleLeft");
		animator.AddClip(L"MoveLeft", L"Player_MoveLeft");
		animator.AddClip(L"IdleRight", L"Player_IdleRight");
		animator.AddClip(L"MoveRight", L"Player_MoveRight");

		player->AddComponent<PlayerAnimationFSMComponent>();
		Rigidbody2DComponent* rigidbody = player->AddComponent<Rigidbody2DComponent>();
		rigidbody->SetLinearDamping(1.f);

		BoxCollider2DComponent* collider = player->AddComponent<BoxCollider2DComponent>();
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
		auto spriteComponent = BackGround->AddComponent<SpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteComponent->SetTexture(texture);
		auto transform = BackGround->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });

		// Monster
		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ (float)200 * i, 300 });
			auto spriteComponent = monster->AddComponent<SpriteComponent>();
			auto texture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			spriteComponent->SetTexture(texture);

			auto transform = monster->GetTransform();
			transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()),static_cast<float>(texture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0, -250 });
		BoxCollider2DComponent* groundCollider = ground->AddComponent<BoxCollider2DComponent>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void MainScene::InitializeCamera(GameObject* player)
	{
		auto cameraObject = SpawnGameObject<GameObject>({ 0, 0 });

		SocketFollowComponent* followComponent = cameraObject->AddComponent<SocketFollowComponent>();
		followComponent->SetTarget(*player, L"Player.Camera");

		auto cameraComponent = cameraObject->AddComponent<CameraComponent>();
		cameraComponent->SetOrthographic(static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()));
		GetCameraManager()->RegisterCamera(L"PlayerCamera", cameraComponent);
	}
}
