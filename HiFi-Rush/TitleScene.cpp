#include "TitleScene.h"
#include "Input.h"
#include "Application.h"
#include "IDebugRenderer.h"
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
		APPLICATION.GetDebugRenderer().RequestDrawText(L"TitleScene", { APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f });
	}
}

