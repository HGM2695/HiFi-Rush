#include "TitleScene.h"
#include "../Engine/Input.h"
#include "../Engine/Application.h"
#include "../Engine/SceneManager.h"

namespace gm
{
	void TitleScene::OnUpdate()
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::M))
			APPLICATION.GetSceneManager().PlayScene(L"MainScene");
	}
}

