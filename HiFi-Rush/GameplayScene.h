#pragma once

#include "Scene.h"

#include <memory>
#include <string>

namespace gm
{
	struct PlayerSpawnDesc;
	class TriggerSequenceSystem;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene();
		~GameplayScene() override;

		TriggerSequenceSystem&			GetTriggerSequenceSystem();
		const TriggerSequenceSystem&	GetTriggerSequenceSystem() const;

	protected:
		bool InitializeMap(const std::wstring& mapResourceKey);
		bool InitializePlayer(const PlayerSpawnDesc& desc);
		void OnUnload() override;

	private:
		std::unique_ptr<TriggerSequenceSystem> _triggerSequenceSystem;
	};
}
