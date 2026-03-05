#include "MainScene.h"
#include "../Engine/GameObject.h"
#include "../Engine/SpriteRenderer.h"

namespace gm
{
	void MainScene::OnInitialize()
	{
		auto player = std::make_unique<GameObject>();

		// 임시 이미지
		SpriteRenderer* spriteRenderer = player->AddComponent<SpriteRenderer>();
		spriteRenderer->ImageLoad(L"Resources/GameObject/orange_mushroom.png");
		AddGameObject(std::move(player));
	}
}

