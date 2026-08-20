#include "TitleScene.h"
#include "Application.h"
#include "AudioStatics.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "GameObject.h"
#include "Input.h"
#include "PhysicsSystem.h"
#include "SceneDebugTools.h"
#include "TitleResources.h"
#include "TitleWidget.h"
#include "UIManager.h"

namespace gm
{
	void TitleScene::OnInitialize()
	{
		GameObject* cameraObject = SpawnGameObject<GameObject>();
		CameraComponent* camera = cameraObject->AddComponent<CameraComponent>();
		GetCameraManager()->RegisterCamera(L"TitleCamera", camera);
	}

	void TitleScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::None);
		APPLICATION.GetInput().SetCursorLocked(false);
		GetCameraManager()->SetActiveCamera(L"TitleCamera");

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();
		uiManager.AddUserWidget<TitleWidget>();

		PlayBGM(TitleResource::BGMKey);
	}

	void TitleScene::OnExit()
	{
		APPLICATION.GetUIManager().ClearViewportWidgets();
	}

	void TitleScene::OnTick(float)
	{
		TickSceneTransitionDebug();
	}
}

