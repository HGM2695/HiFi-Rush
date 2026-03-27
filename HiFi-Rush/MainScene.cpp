#include "MainScene.h"
#include "PlayerMovement.h"
#include "PlayerAnimationFSM.h"
#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/GameObject.h"
#include "../Engine/SpriteRenderer.h"
#include "../Engine/Texture.h"
#include "../Engine/Camera.h"
#include "../Engine/SceneManager.h"
#include "../Engine/SpriteAnimator.h"
#include "../Engine/SpriteAnimationClip.h"
#include "../Engine//Rigidbody2D.h"
#include "../Engine/BoxCollider2D.h"

namespace gm
{
	void MainScene::OnEnter()
	{
		APPLICATION.GetSceneManager().SetPhysicsMode(PhysicsMode::Physics2D);
	}

	void MainScene::OnInitialize()
	{
		InitializeSubObject();
		InitializePlayer();
	}

	void MainScene::InitializePlayer()
	{
		auto player = Instantiate<GameObject>({ 0, 200 });

		auto spriteRenderer = player->AddComponent<SpriteRenderer>();
		spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"PlayerRight"));

		player->AddComponent<PlayerMovement>();

		auto camera = player->AddComponent<Camera>();
		camera->SetDeadZone(400, 300);

		auto spriteAnimator = player->AddComponent<SpriteAnimator>();
		spriteAnimator->AddClip(L"IdleLeft", APPLICATION.GetResources().Find<SpriteAnimationClip>(L"Player_IdleLeft"));
		spriteAnimator->AddClip(L"MoveLeft", APPLICATION.GetResources().Find<SpriteAnimationClip>(L"Player_MoveLeft"));
		spriteAnimator->AddClip(L"IdleRight", APPLICATION.GetResources().Find<SpriteAnimationClip>(L"Player_IdleRight"));
		spriteAnimator->AddClip(L"MoveRight", APPLICATION.GetResources().Find<SpriteAnimationClip>(L"Player_MoveRight"));

		player->AddComponent<PlayerAnimationFSM>();
		Rigidbody2D* rigidbody = player->AddComponent<Rigidbody2D>();
		rigidbody->SetLinearDamping(1.f);

		BoxCollider2D* collider = player->AddComponent<BoxCollider2D>();
		collider->SetSize({ 100.f, 100.f });
	}

	void MainScene::InitializeSubObject()
	{
		//// BackGround
		//auto BackGround = Instantiate<GameObject>({ 0, 0 });
		//spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"Xanadu"));

		// Monster
		for (int i = 0; i < 20; ++i)
		{
			auto monster = Instantiate<GameObject>({ (float)200 * i, 300 });
			auto spriteRenderer = monster->AddComponent<SpriteRenderer>();
			spriteRenderer = monster->AddComponent<SpriteRenderer>();
			spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom"));
		}

		auto ground = Instantiate<GameObject>({ 0, -250 });
		BoxCollider2D* groundCollider = ground->AddComponent<BoxCollider2D>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}
}
