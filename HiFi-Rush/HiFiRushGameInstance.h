#pragma once

#include "GameInstance.h"

namespace gm
{
	class HiFiRushGameInstance final : public GameInstance
	{
	protected:
		bool OnInitialize() override;

	private:
		void SetupScenes();
		void SetupDebugTools();
	};
}
