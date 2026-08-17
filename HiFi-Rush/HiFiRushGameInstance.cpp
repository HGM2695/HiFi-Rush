#include "HiFiRushGameInstance.h"
#include "Application.h"
#include "RhythmBarWidget.h"
#include "LoadResources.h"
#include "Paths.h"
#include "SceneManager.h"
#include "TutorialScene.h"
#include "OutsideScene.h"
#include "QamilScene.h"
#include "TestScene.h"
#include "TitleScene.h"
#include "CommonLoadingScene.h"
#include "GameplayScene.h"
#include "Input.h"
#include "UIManager.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugTextWidget.h"
#include "SceneDebugTools.h"
#endif

namespace gm
{
	namespace
	{
		constexpr float DefaultBPM = 136.f;
		constexpr float InputCalibrationOffsetSeconds = 0.f;
	}

	bool HiFiRushGameInstance::OnInitialize()
	{
		if (_chiAnimationSettings.Load(GetResourcePath(ChiAnimationSettingsFilePath)) == false)
			return false;

		if (LoadResources() == false)
			return false;

		_beatSystem.SetBPM(DefaultBPM);
		_rhythmJudge.SetInputOffset(InputCalibrationOffsetSeconds);
#if GM_ENABLE_DEBUG_TOOLS
		_debugEventPublisher.Initialize();
#endif
		SetupScenes();
		SetupDebugTools();
		return true;
	}

	void HiFiRushGameInstance::OnTick(float)
	{
		_beatSystem.Tick(APPLICATION.GetAudioSystem());

		Input& input = APPLICATION.GetInput();
		Scene* activeScene = APPLICATION.GetSceneManager().GetActiveScene();
		GameplayScene* gameplayScene = dynamic_cast<GameplayScene*>(activeScene);
		if (gameplayScene)
		{
			if (input.IsKeyDown(KeyCode::Tab))
			{
				Widget* rhythmBar = APPLICATION.GetUIManager().FindWidget(RhythmBarWidget::RootWidgetName);
				if (rhythmBar)
					rhythmBar->ToggleVisibility();
			}
			if (input.IsKeyDown(KeyCode::Escape))
				input.SetCursorLocked(input.IsCursorLocked() == false);
		}
#if GM_ENABLE_DEBUG_TOOLS
		_debugEventPublisher.Tick();
#endif
	}

	void HiFiRushGameInstance::SetupScenes()
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

	void HiFiRushGameInstance::SetupDebugTools()
	{
#if GM_ENABLE_DEBUG_TOOLS
		RegisterSceneDebugTools();
		APPLICATION.GetUIManager().AddDebugUserWidget<DebugTextWidget>(_beatSystem);
#endif
	}
}
