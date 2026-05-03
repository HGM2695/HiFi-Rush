#pragma once

#include "EngineCore.h"
#include <string>

namespace gm
{
	#define APPLICATION gm::Application::GetInstance()

	class Input;
	class PhysicsSystem;
	class TimeSystem;
	class SceneManager;
	class Resources;
	class AudioSystem;
	class UIManager;
	class Window;

	struct ApplicationDesc
	{
		HINSTANCE		instance = nullptr;
		std::wstring	title = L"";
		std::wstring	className = L"";
		int				width = 0;
		int				height = 0;
		int				showCommand = 0;
	};

	class Application
	{
	private:
		Application();
		~Application();

	public:
		static Application& GetInstance()
		{
			static Application instance;
			return instance;
		}

	public:
		bool				Initialize(const ApplicationDesc& desc);
		void				Run();

		void				Update();
		void				PhysicsUpdate();
		void				LateUpdate();
		void				Render();
		void				EndFrame();

		void				ShutDownRuntime();
		uint32				GetWidth() const;
		uint32				GetHeight() const;

	public: // subsystem
		Input&				GetInput() { return *_input; }
		PhysicsSystem&		GetPhysicsSystem() { return *_physicsSystem; }
		TimeSystem&			GetTimeSystem() { return *_time; }
		SceneManager&		GetSceneManager() { return *_sceneManager; }
		Resources&			GetResources() { return *_resources; }
		AudioSystem&		GetAudioSystem() { return *_audioSystem; }
		UIManager&			GetUIManager() { return *_uiManager; }

		const Input&		GetInput() const { return *_input; }
		const PhysicsSystem& GetPhysicsSystem() const { return *_physicsSystem; }
		const TimeSystem&	GetTimeSystem() const { return *_time; }
		const SceneManager& GetSceneManager() const { return *_sceneManager; }
		const Resources&	GetResources() const { return *_resources; }
		const AudioSystem&	GetAudioSystem() const { return *_audioSystem; }
		const UIManager&	GetUIManager() const { return *_uiManager; }

	private:
		void				Loop();
		bool				initializeWindow(const ApplicationDesc& desc);
		bool				initializeSubSystem();
		void				createBackDC();

	private:
		HDC							_hDC = nullptr;
		HDC							_backHDC = nullptr;
		HBITMAP						_backBuffer = nullptr;

		std::unique_ptr<Input>			_input;
		std::unique_ptr<PhysicsSystem>	_physicsSystem;
		std::unique_ptr<TimeSystem>		_time;
		std::unique_ptr<SceneManager>	_sceneManager;
		std::unique_ptr<Resources>		_resources;
		std::unique_ptr<AudioSystem>	_audioSystem;
		std::unique_ptr<UIManager>		_uiManager;
		std::unique_ptr<Window>			_window;
	};
}
