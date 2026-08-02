#include "TitleScene.h"
#include "Input.h"
#include "Application.h"
#include "ITextRenderer.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"
#include "BuiltinGraphicsResources.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "GameObject.h"
#include "SceneDebugTools.h"

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
	}

	void TitleScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();

		if (APPLICATION.GetInput().IsKeyDown(KeyCode::M))
			APPLICATION.GetSceneManager().RequestSceneChange(L"TutorialScene", L"CommonLoadingScene");
	}

	void TitleScene::OnRender()
	{
		APPLICATION.GetTextRenderer().RequestDrawText(L"TitleScene", BuiltinResourceKey::DefaultUIFont, { APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f }, 44.f, Colors::White);
	}
}

