#include "Application.h"
#include "WindowsCore.h"
#include "Input.h"
#include "PhysicsSystem.h"
#include "TimeSystem.h"
#include "SceneManager.h"
#include "Resources.h"
#include "Scene.h"
#include "AudioSystem.h"
#include "IGraphicsDevice.h"
#include "IGraphicsResourceFactory.h"
#include "IGraphicsCommandContext.h"
#include "ITextRenderer.h"
#include "UIManager.h"
#include "Window.h"
#include "BuiltinGraphicsResources.h"
#include "Mesh.h"
#include "GraphicsBackend.h"
#include "CameraManager.h"
#include "Renderer.h"
#include "GameInstance.h"
#include <cwchar>

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	Application::Application() = default;
	Application::~Application()
	{
		if (_gameInstance)
			_gameInstance->Shutdown();
	}

	bool Application::Initialize(const ApplicationDesc& desc)
	{
		return Initialize(desc, std::make_unique<GameInstance>());
	}

	bool Application::Initialize(const ApplicationDesc& desc, std::unique_ptr<GameInstance> gameInstance)
	{
		GM_ASSERT_RETURN_VAL(gameInstance, false, "GameInstance가 존재하지 않습니다.");
		_gameInstance = std::move(gameInstance);

		GM_ASSERT_RETURN_VAL(initializeWindow(desc), false, "Window 초기화 실패");
		GM_ASSERT_RETURN_VAL(initializeGraphics(desc), false, "Graphics Device 초기화 실패");
		GM_ASSERT_RETURN_VAL(initializeSubSystem(), false, "SubSystem 초기화 실패");
		GM_ASSERT_RETURN_VAL(initializeBuiltinResources(), false, "Builtin Resource 초기화 실패");
		GM_ASSERT_RETURN_VAL(initializeRenderer(), false, "Renderer 초기화 실패");

		GM_ASSERT_RETURN_VAL(_gameInstance->Initialize(), false, "GameInstance 초기화에 실패했습니다.");

		_backbufferColor = desc.backBufferColor;
		_isFpsDisplayEnabled = desc.showFps;

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
		GM_ASSERT_RETURN_VAL(graphicsBackend.commandContext, false, "Graphics CommandContext 생성에 실패했습니다.");
#if GM_ENABLE_DEBUG_TOOLS
		GM_ASSERT_RETURN_VAL(graphicsBackend.debugRenderer, false, "DebugRenderer 생성에 실패했습니다.");
#endif
		GM_ASSERT_RETURN_VAL(graphicsBackend.textRenderer, false, "TextRenderer 생성에 실패했습니다.");

		_graphicsDevice = std::move(graphicsBackend.device);
		_graphicsResourceFactory = std::move(graphicsBackend.resourceFactory);
		_graphicsCommandContext = std::move(graphicsBackend.commandContext);
#if GM_ENABLE_DEBUG_TOOLS
		_debugRenderer = std::move(graphicsBackend.debugRenderer);
#endif
		_textRenderer = std::move(graphicsBackend.textRenderer);

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
		GM_ASSERT_RETURN_VAL(_renderer->Initialize(), false, "Renderer 초기화에 실패했습니다.");

		return true;
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

		_gameInstance->Shutdown();
	}

	void Application::Loop()
	{
		TickSystems();

		const float deltaTime = _time->GetDeltaTime();
		_sceneManager->BeginFrame();
		_sceneManager->Tick(TickGroup::GameLogic, deltaTime);
		_sceneManager->Tick(TickGroup::Animation, deltaTime);
		_sceneManager->Tick(TickGroup::Movement, deltaTime);
		SimulatePhysics(deltaTime);
		_sceneManager->Tick(TickGroup::PostPhysics, deltaTime);
		_sceneManager->Tick(TickGroup::Attachment, deltaTime);
		_sceneManager->Tick(TickGroup::Camera, deltaTime);
		Render();
		EndFrame();
	}

	void Application::TickSystems()
	{
		_input->Tick();
		_time->Tick();
		_audioSystem->Tick();
		updateFps(_time->GetUnscaledDeltaTime());

		const float deltaTime = _time->GetDeltaTime();
		_gameInstance->Tick(deltaTime);
		_uiManager->Tick(deltaTime);
	}

	void Application::SimulatePhysics(float deltaTime)
	{
		Scene* activeScene = _sceneManager->GetActiveScene();
		GM_ASSERT_RETURN(activeScene, "활성 Scene이 없습니다.");

		_physicsSystem->Simulate(*activeScene, deltaTime);
	}

	void Application::Render()
	{
		_graphicsDevice->BeginFrame(_backbufferColor);

		_sceneManager->Render();
		Scene* activeScene = _sceneManager->GetActiveScene();
		GM_ASSERT_RETURN(activeScene, "활성 Scene이 없습니다.");
		GM_ASSERT_RETURN(activeScene->GetCameraManager(), "CameraManager가 존재하지 않습니다.");

		_uiManager->Render();

		const CameraViewInfo viewInfo = activeScene->GetCameraManager()->GetViewInfo();
#if GM_ENABLE_DEBUG_TOOLS
		_physicsSystem->DebugRender(*activeScene, *_debugRenderer);
		_renderer->DebugDraw(*_debugRenderer);
		_debugRenderer->Render(viewInfo);
#endif

		_renderer->Render(viewInfo, GetWidth(), GetHeight());
		requestDrawFps();
		_textRenderer->Render();

		_graphicsDevice->EndFrame();
	}

	void Application::updateFps(float deltaTime)
	{
		if (_isFpsDisplayEnabled == false)
			return;

		_fpsAccumulatedTime += deltaTime;
		++_fpsFrameCount;

		if (_fpsAccumulatedTime < 1.f)
			return;

		_fps = static_cast<float>(_fpsFrameCount) / _fpsAccumulatedTime;
		_fpsFrameCount = 0;
		_fpsAccumulatedTime = 0.f;
	}

	void Application::requestDrawFps()
	{
		if (_isFpsDisplayEnabled == false)
			return;

		wchar_t text[32]{};
		std::swprintf(text, 32, L"FPS : %.1f", _fps);
		_textRenderer->RequestDrawText(text, BuiltinResourceKey::DefaultUIFont, Vector2{}, 24.f, Colors::Green);
	}

	void Application::EndFrame()
	{
		_sceneManager->EndFrame();
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
