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
#include "UIManager.h"
#include "Window.h"

namespace gm
{
    Application::Application() = default;
    Application::~Application() = default;

    bool Application::Initialize(const ApplicationDesc& desc)
    {
        GM_ASSERT_RETURN_VAL(initializeWindow(desc), false, "Window 초기화 실패");
        createBackDC();
        GM_ASSERT_RETURN_VAL(initializeSubSystem(), false, "SubSystem 초기화 실패");
        
        return true;
    }

    bool Application::initializeWindow(const ApplicationDesc& desc)
    {
        WindowDesc windowDesc;
        windowDesc.className = desc.className;
        windowDesc.title = desc.title;
        windowDesc.instance = desc.instance;
        windowDesc.showCommand = desc.showCommand;
        windowDesc.width = desc.width;
        windowDesc.height = desc.height;

        _window = std::make_unique<Window>();
        GM_ASSERT_RETURN_VAL(_window->Initialize(windowDesc), false, "Window 초기화에 실패했습니다.");

        _hDC = GetDC(_window->GetHandle());
        return true;
    }

    void Application::createBackDC()
    {
        _backHDC = CreateCompatibleDC(_hDC);
        _backBuffer = CreateCompatibleBitmap(_hDC, _window->GetWidth(), _window->GetHeight());

        HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(_backHDC, _backBuffer));
        DeleteObject(oldBitmap);
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
        Update();
        PhysicsUpdate();
        LateUpdate();
        Render();
        EndFrame();
    }

    void Application::Update()
    {
        _input->Update();
        _time->Update();
        _sceneManager->Update();
		_audioSystem->Update();
		_uiManager->Update();
    }

    void Application::LateUpdate()
    {
        _sceneManager->LateUpdate();
    }

	void Application::PhysicsUpdate()
	{
		Scene* activeScene = _sceneManager->GetActiveScene();
		GM_ASSERT_RETURN(activeScene, "활성 Scene이 없습니다.");

		_physicsSystem->Simulate(*activeScene, _time->GetDeltaTime());
	}

    void Application::Render()
    {
        const uint32 width = _window->GetWidth();
        const uint32 height = _window->GetHeight();
        Rectangle(_backHDC, -1, -1, width + 1, height + 1);

        _time->Render(_backHDC);
        _sceneManager->Render(_backHDC);
        debug::DebugRenderer::Render(_backHDC);
		_uiManager->Render(_backHDC);

        BitBlt(_hDC, 0, 0, width, height, _backHDC, 0, 0, SRCCOPY);
    }

    void Application::EndFrame()
    {
        _sceneManager->EndFrame();
    }

    void Application::ShutDownRuntime()
    {
        if (_sceneManager)
            _sceneManager.reset();
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
