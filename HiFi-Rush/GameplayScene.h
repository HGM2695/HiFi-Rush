#pragma once

#include "Scene.h"

#include <memory>

namespace gm
{
	class TriggerSequenceSystem;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene();
		~GameplayScene() override;

		TriggerSequenceSystem& GetTriggerSequenceSystem();
		const TriggerSequenceSystem& GetTriggerSequenceSystem() const;

	protected:
		void OnUnload() override;

	private:
		std::unique_ptr<TriggerSequenceSystem> _triggerSequenceSystem;
	};
}
