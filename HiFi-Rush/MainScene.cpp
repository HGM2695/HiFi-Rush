#include "MainScene.h"
#include "PlayerMovement.h"
#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/GameObject.h"
#include "../Engine/SpriteRenderer.h"
#include "../Engine/Texture.h"

namespace gm
{
	void MainScene::OnInitialize()
	{
		auto player = std::make_unique<GameObject>();

		SpriteRenderer* spriteRenderer = player->AddComponent<SpriteRenderer>();
		spriteRenderer->SetTexture(APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom", ResourceType::Texture));

		PlayerMovement* playerMovement = player->AddComponent<PlayerMovement>();
		
		AddGameObject(std::move(player));
	}
}

