#include "SceneDebugTools.h"

#include "Application.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "DebugInputHandler.h"
#include "GameObject.h"
#include "GMLog.h"
#include "MathUtil.h"
#include "Scene.h"
#include "SceneManager.h"
#include "TransformComponent.h"

namespace gm
{
	constexpr const wchar_t* SceneTransitionDebugType = L"SceneTransition";

#if GM_ENABLE_DEBUG_TOOLS
	namespace
	{
		constexpr const wchar_t* EnvironmentOverviewCameraKey = L"EnvironmentOverviewCamera";
	}
#endif

	void RegisterSceneDebugTools()
	{
#if GM_ENABLE_DEBUG_TOOLS
		DebugInputHandler::RegisterDebugType(SceneTransitionDebugType, true);
#endif
	}

	void TickSceneTransitionDebug()
	{
		const Input& input = APPLICATION.GetInput();
		const bool hasModifier = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl) || input.IsKeyRepeat(KeyCode::LeftShift) || input.IsKeyRepeat(KeyCode::RightShift) || input.IsKeyRepeat(KeyCode::LeftAlt) || input.IsKeyRepeat(KeyCode::RightAlt);
		if (hasModifier)
			return;

		if (input.IsKeyDown(KeyCode::F1))
		{
			APPLICATION.GetSceneManager().RequestSceneChange(
				L"TutorialScene",
				L"CommonLoadingScene"
			);
		}
		else if (input.IsKeyDown(KeyCode::F2))
		{
			APPLICATION.GetSceneManager().RequestSceneChange(
				L"OutsideScene",
				L"CommonLoadingScene"
			);
		}
		else if (input.IsKeyDown(KeyCode::F3))
		{
			APPLICATION.GetSceneManager().RequestSceneChange(
				L"QamilScene",
				L"CommonLoadingScene"
			);
		}
		else if (input.IsKeyDown(KeyCode::F4))
		{
			APPLICATION.GetSceneManager().RequestSceneChange(
				L"TestScene",
				L"CommonLoadingScene"
			);
		}
	}

	void CreateEnvironmentOverviewCamera(Scene& scene, const Vector3& cameraPosition, const Vector3& targetPosition)
	{
#if GM_ENABLE_DEBUG_TOOLS
		GameObject* cameraObject = scene.SpawnGameObject<GameObject>();
		cameraObject->GetTransform()->SetWorldMatrix(Math::CreateLookAtLH(cameraPosition, targetPosition, Vector3::Up).Invert());

		CameraComponent* camera = cameraObject->AddComponent<CameraComponent>();
		const float aspectRatio = static_cast<float>(APPLICATION.GetWidth()) / static_cast<float>(APPLICATION.GetHeight());
		camera->SetPerspective(Math::GM_PI / 3.f, aspectRatio, 0.1f, 500.f);
		scene.GetCameraManager()->RegisterCamera(EnvironmentOverviewCameraKey, camera);
#else
		(void)scene;
		(void)cameraPosition;
		(void)targetPosition;
#endif
	}

	void ActivateEnvironmentOverviewCamera(Scene& scene)
	{
#if GM_ENABLE_DEBUG_TOOLS
		scene.GetCameraManager()->SetActiveCamera(EnvironmentOverviewCameraKey);
#else
		(void)scene;
#endif
	}
}
