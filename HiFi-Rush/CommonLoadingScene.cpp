#include "CommonLoadingScene.h"
#include "Application.h"
#include "ITextRenderer.h"
#include "SceneManager.h"
#include "Input.h"

namespace gm
{
	void CommonLoadingScene::OnEnter()
	{	
		_pendingSceneName = APPLICATION.GetSceneManager().GetPendingSceneName();
		GM_ASSERT_RETURN(_pendingSceneName.empty() == false, "로딩 씬을 호출했는데, pendingScene이 존재하지 않습니다.");
	}

	void CommonLoadingScene::OnTick(float deltaTime)
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::S))
			APPLICATION.GetSceneManager().RequestSceneChange(_pendingSceneName);
	}

	void CommonLoadingScene::OnRender()
	{
		APPLICATION.GetTextRenderer().RequestDrawText(L"Loading Scene...", L"Engine.DefaultUI", Vector2(APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f), 24.f, Colors::White);
	}
}
