#include "GameplayScene.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "ComboResultWidget.h"
#include "GameplayAnnouncementWidget.h"
#include "RhythmBarWidget.h"
#include "RhythmMeterWidget.h"
#include "RhythmRankComponent.h"
#include "EnvironmentSpawner.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "HealthComponent.h"
#include "HiFiRushStatics.h"
#include "MapResource.h"
#include "MonsterSpawner.h"
#include "PlayerSpawner.h"
#include "PlayerStatusWidget.h"
#include "Resources.h"
#include "ReverbComponent.h"
#include "TriggerSequenceSystem.h"
#include "UIManager.h"

#include <memory>

namespace gm
{
	GameplayScene::GameplayScene()
		: _triggerSequenceSystem(std::make_unique<TriggerSequenceSystem>(HiFiRushStatics::GetBeatSystem()))
	{}

	GameplayScene::~GameplayScene() = default;

	TriggerSequenceSystem& GameplayScene::GetTriggerSequenceSystem()
	{
		return *_triggerSequenceSystem;
	}

	const TriggerSequenceSystem& GameplayScene::GetTriggerSequenceSystem() const
	{
		return *_triggerSequenceSystem;
	}

	void GameplayScene::PlayAnnouncement(GameplayAnnouncementType type)
	{
		GM_ASSERT_RETURN(_announcementWidget, "Gameplay Announcement Widget이 구성되지 않았습니다.");
		_announcementWidget->Play(type);
	}

	bool GameplayScene::InitializeMap(const std::wstring& mapResourceKey)
	{
		const std::shared_ptr<MapResource> mapResource = APPLICATION.GetResources().Find<MapResource>(mapResourceKey);
		GM_ASSERT_RETURN_VAL(mapResource, false, "MapResource가 로드되지 않았습니다. key=%ls", mapResourceKey.c_str());

		const MapData& mapData = mapResource->GetData();

		EnvironmentSpawner environmentSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(environmentSpawner.Spawn(*this, mapData.objects), false, "환경 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

		MonsterSpawner monsterSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(monsterSpawner.Spawn(*this, mapData.monsterSpawnDatas), false, "Monster 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

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

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();
		uiManager.AddUserWidget<PlayerStatusWidget>(HiFiRushStatics::GetBeatSystem(), *healthComponent, *reverbComponent, *stateMachine);
		uiManager.AddUserWidget<RhythmMeterWidget>(HiFiRushStatics::GetBeatSystem(), *rhythmRankComponent);
		uiManager.AddUserWidget<RhythmBarWidget>(HiFiRushStatics::GetBeatSystem());
		uiManager.AddUserWidget<ComboResultWidget>(*stateMachine);
		_announcementWidget = uiManager.AddUserWidget<GameplayAnnouncementWidget>(HiFiRushStatics::GetBeatSystem());
	}

	void GameplayScene::OnUnload()
	{
		_player.Reset();
		_announcementWidget = nullptr;
		_triggerSequenceSystem->Clear();
	}
}
