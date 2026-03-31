#include "CommonLoadingScene.h"
#include "../Engine/GMAssert.h"
#include "../Engine/Application.h"
#include "../Engine/SceneManager.h"
#include "../Engine/Input.h"
#include "../Engine/DebugRenderer.h"

namespace gm
{
	void CommonLoadingScene::OnEnter()
	{	
		_pendingSceneName = APPLICATION.GetSceneManager().GetPendingSceneName();
		GM_ASSERT_RETURN(_pendingSceneName.empty() == false, "로딩 씬을 호출했는데, pendingScene이 존재하지 않습니다.");
	}

	void CommonLoadingScene::OnUpdate()
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::S))
			APPLICATION.GetSceneManager().RequestSceneChange(_pendingSceneName);
	}

	void CommonLoadingScene::OnRender(HDC hDC)
	{
		debug::DebugRenderer::RequestDrawText(L"Loading Scene...", math::Vector2(APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f));
	}
}
