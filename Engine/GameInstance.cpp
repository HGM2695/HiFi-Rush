#include "GameInstance.h"

namespace gm
{
	bool GameInstance::Initialize()
	{
		if (_isInitialized)
			return true;

		if (OnInitialize() == false)
			return false;

		_isInitialized = true;
		return true;
	}

	void GameInstance::Tick(float deltaTime)
	{
		if (_isInitialized)
			OnTick(deltaTime);
	}

	void GameInstance::Shutdown()
	{
		if (_isInitialized == false)
			return;

		OnShutdown();
		_isInitialized = false;
	}
}
