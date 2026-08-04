#pragma once

#include "EngineCore.h"

namespace gm
{
	class Application;

	class GameInstance
	{
	friend class Application;

	public:
		virtual ~GameInstance() = default;

	protected:
		virtual bool OnInitialize() { return true; }
		virtual void OnTick(float deltaTime) {}
		virtual void OnShutdown() {}

	private:
		bool Initialize();
		void Tick(float deltaTime);
		void Shutdown();

	private:
		bool _isInitialized = false;
	};
}
