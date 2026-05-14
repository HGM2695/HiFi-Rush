#include "TitleScene.h"
#include "Input.h"
#include "Application.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"
#include "DebugRenderer.h"

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

	void TitleScene::OnRender()
	{
		debug::DebugRenderer::RequestDrawText(L"TitleScene", { APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f });
	}
}

