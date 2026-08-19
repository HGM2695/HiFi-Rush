#include "GameplayScene.h"
#include "Application.h"
#include "AudioComponent.h"
#include "ChiStateMachineComponent.h"
#include "ComboResultWidget.h"
#include "DialogComponent.h"
#include "DialogWidget.h"
#include "GameplayAnnouncementWidget.h"
#include "RhythmBarWidget.h"
#include "RhythmMeterWidget.h"
#include "RhythmRankComponent.h"
#include "EnvironmentSpawner.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "HealthComponent.h"
#include "HiFiRushStatics.h"
#include "HiFiRushDialogData.h"
#include "MapResource.h"
#include "MonsterSpawner.h"
#include "PlayerControlComponent.h"
#include "PlayerSpawner.h"
#include "PlayerStatusWidget.h"
#include "Resources.h"
#include "ReverbComponent.h"
#include "SoundWave.h"
#include "TriggerSystem.h"
#include "UIManager.h"

#include <memory>

namespace gm
{
	GameplayScene::GameplayScene()
		: _triggerSystem(std::make_unique<TriggerSystem>(HiFiRushStatics::GetBeatSystem()))
	{}

	GameplayScene::~GameplayScene() = default;

	TriggerSystem& GameplayScene::GetTriggerSystem()
	{
		return *_triggerSystem;
	}

	const TriggerSystem& GameplayScene::GetTriggerSystem() const
	{
		return *_triggerSystem;
	}

	DialogComponent& GameplayScene::GetDialogComponent()
	{
		GM_ASSERT_TERMINATE(_dialogComponent, "DialogComponent가 구성되지 않았습니다.");
		return *_dialogComponent;
	}

	const DialogComponent& GameplayScene::GetDialogComponent() const
	{
		GM_ASSERT_TERMINATE(_dialogComponent, "DialogComponent가 구성되지 않았습니다.");
		return *_dialogComponent;
	}

	bool GameplayScene::PlayDialogSequence(const std::wstring& sequenceId)
	{
		GM_ASSERT_RETURN_VAL(_dialogComponent, false, "DialogComponent가 구성되지 않았습니다.");
		return _dialogComponent->PlaySequence(sequenceId);
	}

	bool GameplayScene::SelectDialogBranch(const std::wstring& branchKey)
	{
		GM_ASSERT_RETURN_VAL(_dialogComponent, false, "DialogComponent가 구성되지 않았습니다.");
		return _dialogComponent->SelectBranch(branchKey);
	}

	void GameplayScene::PlayAnnouncement(GameplayAnnouncementType type)
	{
		GM_ASSERT_RETURN(_announcementWidget, "Gameplay Announcement Widget이 구성되지 않았습니다.");
		_announcementWidget->Play(type);
	}

	void GameplayScene::SetGameplayStatusUIVisible(bool isVisible)
	{
		GM_ASSERT_RETURN(_playerStatusWidget && _rhythmMeterWidget, "Gameplay Status UI가 구성되지 않았습니다.");
		_playerStatusWidget->SetVisible(isVisible);
		_rhythmMeterWidget->SetVisible(isVisible);
	}

	bool GameplayScene::InitializeMap(const std::wstring& mapResourceKey)
	{
		std::vector<MonsterSpawnResult> unusedMonsterSpawnResults;
		return InitializeMap(mapResourceKey, unusedMonsterSpawnResults);
	}

	bool GameplayScene::InitializeMap(const std::wstring& mapResourceKey, std::vector<MonsterSpawnResult>& outMonsterSpawnResults)
	{
		const std::shared_ptr<MapResource> mapResource = APPLICATION.GetResources().Find<MapResource>(mapResourceKey);
		GM_ASSERT_RETURN_VAL(mapResource, false, "MapResource가 로드되지 않았습니다. key=%ls", mapResourceKey.c_str());

		const MapData& mapData = mapResource->GetData();

		EnvironmentSpawner environmentSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(environmentSpawner.Spawn(*this, mapData.objects), false, "환경 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

		MonsterSpawner monsterSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(monsterSpawner.SpawnAll(*this, mapData.monsterSpawnDatas, outMonsterSpawnResults), false, "Monster 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

		return true;
	}

	bool GameplayScene::InitializePlayer(const PlayerSpawnDesc& desc)
	{
		PlayerSpawner playerSpawner(APPLICATION.GetResources());
		GameObject* player = playerSpawner.Spawn(*this, desc, HiFiRushStatics::GetPlayerRuntimeState());
		if (player == nullptr)
			return false;

		_player = player->GetWeakPtr();
		return true;
	}

	void GameplayScene::InitializeGameplayUI()
	{
		GameObject* player = _player.Get();
		GM_ASSERT_RETURN(player, "Gameplay UI를 구성하려면 Player가 필요합니다.");

		HealthComponent* healthComponent = player->GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(healthComponent, "PlayerStatusWidget을 구성하려면 Player HealthComponent가 필요합니다.");
		ReverbComponent* reverbComponent = player->GetComponent<ReverbComponent>();
		GM_ASSERT_RETURN(reverbComponent, "PlayerStatusWidget을 구성하려면 Player ReverbComponent가 필요합니다.");
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(stateMachine, "PlayerStatusWidget을 구성하려면 ChiStateMachineComponent가 필요합니다.");
		RhythmRankComponent* rhythmRankComponent = player->GetComponent<RhythmRankComponent>();
		GM_ASSERT_RETURN(rhythmRankComponent, "RhythmMeterWidget을 구성하려면 RhythmRankComponent가 필요합니다.");
		PlayerControlComponent* playerControlComponent = player->GetComponent<PlayerControlComponent>();
		GM_ASSERT_RETURN(playerControlComponent, "Dialog을 구성하려면 PlayerControlComponent가 필요합니다.");

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();

		GameObject* dialogObject = SpawnGameObject<GameObject>();
		AudioComponent* dialogAudio = dialogObject->AddComponent<AudioComponent>(std::shared_ptr<SoundWave>{});
		GM_ASSERT_RETURN(dialogAudio, "Dialog AudioComponent 생성에 실패했습니다.");
		_dialogComponent = dialogObject->AddComponent<DialogComponent>(*dialogAudio, *playerControlComponent);
		GM_ASSERT_RETURN(_dialogComponent, "DialogComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN(_dialogComponent->SetSequences(CreateHiFiRushDialogSequences()), "Hi-Fi RUSH Dialog Sequence 구성에 실패했습니다.");

		_playerStatusWidget = uiManager.AddUserWidget<PlayerStatusWidget>(HiFiRushStatics::GetBeatSystem(), *healthComponent, *reverbComponent, *stateMachine);
		_rhythmMeterWidget = uiManager.AddUserWidget<RhythmMeterWidget>(HiFiRushStatics::GetBeatSystem(), *rhythmRankComponent);
		uiManager.AddUserWidget<RhythmBarWidget>(HiFiRushStatics::GetBeatSystem());
		uiManager.AddUserWidget<ComboResultWidget>(*stateMachine);
		_announcementWidget = uiManager.AddUserWidget<GameplayAnnouncementWidget>(HiFiRushStatics::GetBeatSystem());
		uiManager.AddUserWidget<DialogWidget>(HiFiRushStatics::GetBeatSystem(), *_dialogComponent);
	}

	void GameplayScene::OnUnload()
	{
		_player.Reset();
		_dialogComponent = nullptr;
		_announcementWidget = nullptr;
		_playerStatusWidget = nullptr;
		_rhythmMeterWidget = nullptr;
		_triggerSystem->Clear();
	}
}
