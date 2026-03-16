#include "MainScene.h"
#include "PlayerMovement.h"
#include "PlayerAnimationFSM.h"
#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/GameObject.h"
#include "../Engine/SpriteRenderer.h"
#include "../Engine/Texture.h"
#include "../Engine/Camera.h"
#include "../Engine/SpriteAnimator.h"
#include "../Engine/SpriteAnimationClip.h"

namespace gm
{
	void MainScene::OnInitialize()
	{
		InitializeSubObject();
		InitializePlayer();
	}

	void MainScene::InitializePlayer()
	{
		auto player = Instantiate<GameObject>({0, 0});

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
	}

	void MainScene::InitializeSubObject()
	{
		// BackGround
		auto BackGround = Instantiate<GameObject>({ 0, 0 });
		auto spriteRenderer = BackGround->AddComponent<SpriteRenderer>();
		spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"Xanadu"));

		// Monster
		auto monster = Instantiate<GameObject>({ 200, 300 });
		spriteRenderer = monster->AddComponent<SpriteRenderer>();
		spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom"));
	}
}
