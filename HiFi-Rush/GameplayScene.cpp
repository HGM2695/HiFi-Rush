#include "GameplayScene.h"
#include "Application.h"
#include "EnvironmentSpawner.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "HiFiRushStatics.h"
#include "MapResource.h"
#include "MonsterSpawner.h"
#include "PlayerSpawner.h"
#include "Resources.h"
#include "TriggerSequenceSystem.h"

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

	void GameplayScene::OnUnload()
	{
		_player.Reset();
		_triggerSequenceSystem->Clear();
	}
}
