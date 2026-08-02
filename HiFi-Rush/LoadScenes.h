#pragma once

#include "SceneManager.h"
#include "Application.h"
#include "TutorialScene.h"
#include "OutsideScene.h"
#include "QamilScene.h"
#include "TestScene.h"
#include "TitleScene.h"
#include "CommonLoadingScene.h"

namespace gm
{
	void SetupScenes()
	{
		SceneManager& sceneManager = APPLICATION.GetSceneManager();
		sceneManager.CreateScene<TitleScene>(L"TitleScene");
		sceneManager.CreateScene<TutorialScene>(L"TutorialScene");
		sceneManager.CreateScene<OutsideScene>(L"OutsideScene");
		sceneManager.CreateScene<QamilScene>(L"QamilScene");
		sceneManager.CreateScene<TestScene>(L"TestScene");
		sceneManager.CreateScene<CommonLoadingScene>(L"CommonLoadingScene");

		sceneManager.RequestSceneChange(L"TitleScene");
	}
}
