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

namespace gm
{
	void MainScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics2D);
		APPLICATION.GetUIManager().ClearViewportWidgets();
		APPLICATION.GetUIManager().AddWidget<MainHUDWidget>();
	}

	void MainScene::OnInitialize()
	{
		//InitializeSubObject();
		InitializePlayer();
	}

	void MainScene::InitializePlayer()
	{
		auto player = SpawnGameObject<GameObject>({ 0, 200 });

		auto transform = player->GetTransform();
		transform->SetScale(Vector2{ 60.f, 120.f });

		//MaterialDesc materialDesc{};
		//materialDesc.pipelineState = APPLICATION.GetResources().Find<PipelineState>(L"OrangeMushroom");
		auto spriteRenderer = player->AddComponent<SpriteRenderer>();
		//spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"PlayerRight"));

		player->AddComponent<PlayerMovement>();

		auto camera = player->AddComponent<Camera>();
		camera->SetOrthographic(static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()));
		GetCameraManager()->RegisterCamera(L"PlayerCamera", camera);
		GetCameraManager()->SetActiveCamera(L"PlayerCamera");

		auto spriteAnimator = player->AddComponent<SpriteAnimator>();
		spriteAnimator->AddClip(L"IdleLeft", L"Player_IdleLeft");
		spriteAnimator->AddClip(L"MoveLeft", L"Player_MoveLeft");
		spriteAnimator->AddClip(L"IdleRight", L"Player_IdleRight");
		spriteAnimator->AddClip(L"MoveRight", L"Player_MoveRight");

		player->AddComponent<PlayerAnimationFSM>();
		Rigidbody2D* rigidbody = player->AddComponent<Rigidbody2D>();
		rigidbody->SetLinearDamping(1.f);

		BoxCollider2D* collider = player->AddComponent<BoxCollider2D>();
		collider->SetSize({ 100.f, 100.f });

		//WidgetComponent* userWidget = player->AddComponent<WidgetComponent>();
		//userWidget->SetWorldOffset({ -175.f, 200.f });
		//userWidget->CreateUserWidget<MainHUDWidget>();
	}

	void MainScene::InitializeSubObject()
	{
		//// BackGround
		//auto BackGround = SpawnGameObject<GameObject>({ 0, 0 });
		//spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"Xanadu"));

		// Monster
		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ (float)200 * i, 300 });
			auto spriteRenderer = monster->AddComponent<SpriteRenderer>();
			spriteRenderer = monster->AddComponent<SpriteRenderer>();
			spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom"));
		}

		auto ground = SpawnGameObject<GameObject>({ 0, -250 });
		BoxCollider2D* groundCollider = ground->AddComponent<BoxCollider2D>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}
}
