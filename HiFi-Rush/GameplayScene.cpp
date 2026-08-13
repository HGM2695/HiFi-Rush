#include "GameplayScene.h"
#include "HiFiRushStatics.h"
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

	void GameplayScene::OnUnload()
	{
		_triggerSequenceSystem->Clear();
	}
}
