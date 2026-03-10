#include "MainScene.h"
#include "PlayerMovement.h"
#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/GameObject.h"
#include "../Engine/SpriteRenderer.h"
#include "../Engine/Texture.h"
#include "../Engine/Camera.h"

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
		spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom"));

		player->AddComponent<PlayerMovement>();

		auto camera = player->AddComponent<Camera>();
		camera->SetDeadZone(400, 300);
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