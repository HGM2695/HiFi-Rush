#include "MainScene.h"
#include "Player.h"

namespace gm
{
	void MainScene::OnInitialize()
	{
		auto player = std::make_unique<Player>();
		AddGameObject(std::move(player));
	}
}

