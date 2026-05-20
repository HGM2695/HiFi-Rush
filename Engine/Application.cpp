#include "Application.h"
#include "WindowsCore.h"
#include "Input.h"
#include "PhysicsSystem.h"
#include "TimeSystem.h"
#include "SceneManager.h"
#include "Resources.h"
#include "Scene.h"
#include "AudioSystem.h"
#include "DebugRenderer.h"
#include "IGraphicsDevice.h"
#include "IGraphicsResourceFactory.h"
#include "IGraphicsCommandContext.h"
#include "UIManager.h"
#include "Window.h"
#include "BuiltinGraphicsResources.h"
#include "Mesh.h"
#include "PipelineState.h"
#include "GraphicsBackend.h"
#include "CameraManager.h"
#include "Renderer.h"

namespace gm
{
    Application::Application() = default;
    Application::~Application() = default;

    bool Application::Initialize(const ApplicationDesc& desc)
    {
        GM_ASSERT_RETURN_VAL(initializeWindow(desc), false, "Window 초기화 실패");
        GM_ASSERT_RETURN_VAL(initializeGraphics(desc), false, "Graphics Device 초기화 실패");
        GM_ASSERT_RETURN_VAL(initializeSubSystem(), false, "SubSystem 초기화 실패");
        GM_ASSERT_RETURN_VAL(initializeBuiltinResources(), false, "builtin Resource 초기화 실패");
        GM_ASSERT_RETURN_VAL(initializeRenderer(), false, "Renderer 초기화 실패");
        
        return true;
    }

    bool Application::initializeWindow(const ApplicationDesc& desc)
    {
        WindowDesc windowDesc{};
        windowDesc.className = desc.className;
        windowDesc.title = desc.title;
        windowDesc.instance = desc.instance;
        windowDesc.showCommand = desc.showCommand;
        windowDesc.width = desc.width;
        windowDesc.height = desc.height;

        _window = std::make_unique<Window>();
        GM_ASSERT_RETURN_VAL(_window->Initialize(windowDesc), false, "Window 초기화에 실패했습니다.");

        return true;
    }

    bool Application::initializeGraphics(const ApplicationDesc& desc)
    {
        GraphicsBackendDesc graphicsDesc{};
        graphicsDesc.graphicsAPI = desc.graphicsAPI;
        graphicsDesc.width = desc.width;
        graphicsDesc.height = desc.height;
        graphicsDesc.hWnd = _window->GetHandle();
        graphicsDesc.isVSync = desc.isVSync;

        GraphicsBackend graphicsBackend = CreateGraphicsBackend(graphicsDesc);
        GM_ASSERT_RETURN_VAL(graphicsBackend.device, false, "그래픽 디바이스 생성에 실패했습니다.");
        GM_ASSERT_RETURN_VAL(graphicsBackend.resourceFactory, false, "그래픽 리소스 팩토리 생성에 실패했습니다.");

        _graphicsDevice = std::move(graphicsBackend.device);
        _graphicsResourceFactory = std::move(graphicsBackend.resourceFactory);
        _graphicsCommandContext = std::move(graphicsBackend.commandContext);

        return true;
    }

    bool Application::initializeSubSystem()
    {
        _input = std::make_unique<Input>(_window->GetHandle());

		_physicsSystem = std::make_unique<PhysicsSystem>();

        _time = std::make_unique<TimeSystem>();

        _sceneManager = std::make_unique<SceneManager>();

        _resources = std::make_unique<Resources>();

		_audioSystem = std::make_unique<AudioSystem>();
		GM_ASSERT_RETURN_VAL(_audioSystem->Initialize(), false, "AudioSystem 초기화에 실패했습니다.");

		_uiManager = std::make_unique<UIManager>();

        return true;
    }

    bool Application::initializeBuiltinResources()
    {
        return BuiltinGraphicsResources::Load(*_resources, *_graphicsResourceFactory);
    }

    bool Application::initializeRenderer()
    {
        _renderer = std::make_unique<Renderer>(*_resources, *_graphicsCommandContext, *_graphicsResourceFactory);
        return _renderer->Initialize();
    }

    void Application::Run()
    {
        MSG msg;

        while (true)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    break;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                Loop();
            }
        }

        APPLICATION.ShutDownRuntime();
    }

    void Application::Loop()
    {
		const float deltaTime = TickSystems();

		_sceneManager->BeginFrame();
		_sceneManager->Tick(TickGroup::PrePhysics, deltaTime);
		_sceneManager->Tick(TickGroup::GameLogic, deltaTime);
		SimulatePhysics(deltaTime);
		_sceneManager->Tick(TickGroup::PostPhysics, deltaTime);
		_sceneManager->Tick(TickGroup::Attachment, deltaTime);
		_sceneManager->Tick(TickGroup::Camera, deltaTime);
		_sceneManager->Tick(TickGroup::RenderSubmit, deltaTime);
        Render();
        EndFrame();
    }

    float Application::TickSystems()
    {
        _input->Tick();
        _time->Tick();
		_audioSystem->Tick();

		const float deltaTime = _time->GetDeltaTime();
		_uiManager->Tick(deltaTime);

		return deltaTime;
    }

	void Application::SimulatePhysics(float deltaTime)
	{
		Scene* activeScene = _sceneManager->GetActiveScene();
		GM_ASSERT_RETURN(activeScene, "활성 Scene이 없습니다.");

		_physicsSystem->Simulate(*activeScene, deltaTime);
	}

    void Application::Render()
    {
        static const Color backBufferColor = Colors::Magenta;
        _graphicsDevice->BeginFrame(backBufferColor);

        _sceneManager->Render();
		Scene* activeScene = _sceneManager->GetActiveScene();
		GM_ASSERT_RETURN(activeScene, "활성 Scene이 없습니다.");
		GM_ASSERT_RETURN(activeScene->GetCameraManager(), "CameraManager가 존재하지 않습니다.");

		_renderer->Render(activeScene->GetCameraManager()->GetViewInfo());
        debug::DebugRenderer::Render();
		_uiManager->Render();

        _graphicsDevice->EndFrame();
    }

    void Application::EndFrame()
    {
        _sceneManager->EndFrame();
    }

    void Application::ShutDownRuntime()
    {
        if (_sceneManager)
            _sceneManager.reset();
		if (_renderer)
			_renderer.reset();
        if (_resources)
            _resources.reset();
		if (_physicsSystem)
			_physicsSystem.reset();
		if (_audioSystem)
			_audioSystem.reset();
		if (_uiManager)
			_uiManager.reset();
    }

    uint32 Application::GetWidth() const
    {
        return _window->GetWidth();
    }

    uint32 Application::GetHeight() const
    {
        return _window->GetHeight();
    }
}
