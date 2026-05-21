#include "TitleScene.h"
#include "Input.h"
#include "Application.h"
#include "ITextRenderer.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"

namespace gm
{
	void TitleScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::None);
	}

	void TitleScene::OnTick(float deltaTime)
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::M))
			APPLICATION.GetSceneManager().RequestSceneChange(L"MainScene");
	}

	void TitleScene::OnRender()
	{
		APPLICATION.GetTextRenderer().RequestDrawText(L"TitleScene", L"Engine.DefaultUI", { APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f }, 44.f, Colors::White);
	}
}

