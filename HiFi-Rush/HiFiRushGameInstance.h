#pragma once

#include "GameInstance.h"
#include "BeatSystem.h"

namespace gm
{
	class HiFiRushGameInstance final : public GameInstance
	{
	public:
		BeatSystem& GetBeatSystem() { return _beatSystem; }
		const BeatSystem& GetBeatSystem() const { return _beatSystem; }

	protected:
		bool OnInitialize() override;
		void OnTick(float) override;

	private:
		void SetupScenes();
		void SetupDebugTools();

	private:
		BeatSystem _beatSystem;
	};
}
