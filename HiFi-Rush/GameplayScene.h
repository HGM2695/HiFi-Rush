#pragma once

#include "Scene.h"
#include "WeakGameObjectPtr.h"

#include <memory>
#include <string>

namespace gm
{
	struct PlayerSpawnDesc;
	class TriggerSequenceSystem;
	class GameplayAnnouncementWidget;
	enum class GameplayAnnouncementType;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene();
		~GameplayScene() override;

		TriggerSequenceSystem&			GetTriggerSequenceSystem();
		const TriggerSequenceSystem&	GetTriggerSequenceSystem() const;
		const WeakGameObjectPtr&		GetPlayer() const { return _player; }
		void PlayAnnouncement(GameplayAnnouncementType type);

	protected:
		bool InitializeMap(const std::wstring& mapResourceKey);
		bool InitializePlayer(const PlayerSpawnDesc& desc);
		void InitializeGameplayUI();
		void OnUnload() override;

	private:
		std::unique_ptr<TriggerSequenceSystem>	_triggerSequenceSystem;
		WeakGameObjectPtr						_player{};
		GameplayAnnouncementWidget*			_announcementWidget = nullptr;
	};
}
