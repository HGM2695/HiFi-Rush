#include "TitleScene.h"
#include "../Engine/Input.h"
#include "../Engine/Application.h"
#include "../Engine/PhysicsSystem.h"
#include "../Engine/SceneManager.h"
#include "../Engine/DebugRenderer.h"

namespace gm
{
	void TitleScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::None);
	}

	void TitleScene::OnUpdate()
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::M))
			APPLICATION.GetSceneManager().RequestSceneChange(L"MainScene");
	}

	void TitleScene::OnRender(HDC hDC)
	{
		debug::DebugRenderer::RequestDrawText(L"TitleScene", { APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f });
	}
}

