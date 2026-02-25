#include "MainScene.h"
#include "../Engine/GameObject.h"

namespace gm
{
	void MainScene::OnInitialize()
	{
		auto player = std::make_unique<GameObject>();
		AddGameObject(std::move(player));
	}
}

