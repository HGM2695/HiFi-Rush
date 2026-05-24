#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

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
	class IGraphicsDevice;
	class IGraphicsResourceFactory;
	class IGraphicsCommandContext;
	class Renderer;
	class ITextRenderer;

#if GM_ENABLE_DEBUG_TOOLS
	class IDebugRenderer;
#endif

	struct ApplicationDesc
	{
		HINSTANCE		instance = nullptr;
		std::wstring	title = L"";
		std::wstring	className = L"";
		int				width = 0;
		int				height = 0;
		int				showCommand = 0;
		bool			isVSync = true;
		GraphicsAPI		graphicsAPI = GraphicsAPI::D3D11;
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

		float				TickSystems();
		void				SimulatePhysics(float deltaTime);
		void				Render();
		void				EndFrame();

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
		IGraphicsDevice&	GetGraphicsDevice() { return *_graphicsDevice; }
		IGraphicsResourceFactory& GetGraphicsResourceFactory() { return *_graphicsResourceFactory; }
		IGraphicsCommandContext& GetGraphicsCommandContext() { return *_graphicsCommandContext; }
		Renderer&			GetRenderer() { return *_renderer; }
		ITextRenderer&		GetTextRenderer() { return *_textRenderer; }

#if GM_ENABLE_DEBUG_TOOLS
		IDebugRenderer& GetDebugRenderer() { return *_debugRenderer; }
#endif

		const Input&		GetInput() const { return *_input; }
		const PhysicsSystem& GetPhysicsSystem() const { return *_physicsSystem; }
		const TimeSystem&	GetTimeSystem() const { return *_time; }
		const SceneManager& GetSceneManager() const { return *_sceneManager; }
		const Resources&	GetResources() const { return *_resources; }
		const AudioSystem&	GetAudioSystem() const { return *_audioSystem; }
		const UIManager&	GetUIManager() const { return *_uiManager; }
		const IGraphicsDevice& GetGraphicsDevice() const { return *_graphicsDevice; }
		const IGraphicsResourceFactory& GetGraphicsResourceFactory() const { return *_graphicsResourceFactory; }
		const IGraphicsCommandContext& GetGraphicsCommandContext() const { return *_graphicsCommandContext; }
		const Renderer&		GetRenderer() const { return *_renderer; }
		const ITextRenderer& GetTextRenderer() const { return *_textRenderer; }

#if GM_ENABLE_DEBUG_TOOLS
		const IDebugRenderer& GetDebugRenderer() const { return *_debugRenderer; }
#endif

	private:
		void				Loop();
		bool				initializeWindow(const ApplicationDesc& desc);
		bool				initializeGraphics(const ApplicationDesc& desc);
		bool				initializeSubSystem();
		bool				initializeBuiltinResources();
		bool				initializeRenderer();

	private:
		std::unique_ptr<Window>                      _window;
		std::unique_ptr<IGraphicsDevice>             _graphicsDevice;
		std::unique_ptr<IGraphicsCommandContext>     _graphicsCommandContext;
		std::unique_ptr<IGraphicsResourceFactory>    _graphicsResourceFactory;

		std::unique_ptr<ITextRenderer>               _textRenderer;
#if GM_ENABLE_DEBUG_TOOLS
		std::unique_ptr<IDebugRenderer>              _debugRenderer;
#endif
		std::unique_ptr<Renderer>                    _renderer;

		std::unique_ptr<AudioSystem>                 _audioSystem;
		std::unique_ptr<Resources>                   _resources;
		std::unique_ptr<UIManager>                   _uiManager;
		std::unique_ptr<SceneManager>                _sceneManager;

		std::unique_ptr<PhysicsSystem>               _physicsSystem;
		std::unique_ptr<Input>                       _input;
		std::unique_ptr<TimeSystem>                  _time;
	};
}
