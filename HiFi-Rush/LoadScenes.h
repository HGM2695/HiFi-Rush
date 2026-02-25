#pragma once

#include "../Engine/SceneManager.h"
#include "../Engine/Application.h"
#include "MainScene.h"

namespace gm
{
	void SetupScenes()
	{
		SceneManager& sceneManager = APPLICATION.GetSceneManager();
		sceneManager.CreateScene<MainScene>(L"MainScene");
		sceneManager.PlayScene(L"MainScene");
	}
}