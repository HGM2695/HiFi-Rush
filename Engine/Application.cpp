#include "Application.h"
#include "windows.h"
#include "Input.h"
#include "PhysicsSystem.h"
#include "Time.h"
#include "SceneManager.h"
#include "Resources.h"
#include "Scene.h"
#include "AudioSystem.h"
#include "DebugRenderer.h"
#include "UIManager.h"

namespace gm
{
    Application::Application() = default;
    Application::~Application() = default;

    void Application::Initialize(HWND hWnd, uint32 width, uint32 height)
    {
        initializeWindow(hWnd, width, height);
        createBackDC();
        initializeSubSystem();
    }

    void Application::initializeWindow(HWND hWnd, uint32 width, uint32 height)
    {
        _hWnd = hWnd;
        _hDC = GetDC(_hWnd);

        _width = width;
        _height = height;
    }

    void Application::createBackDC()
    {
        _backHDC = CreateCompatibleDC(_hDC);
        _backBuffer = CreateCompatibleBitmap(_hDC, _width, _height);

        HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(_backHDC, _backBuffer));
        DeleteObject(oldBitmap);
    }

    void Application::initializeSubSystem()
    {
        _input = std::make_unique<Input>(_hWnd);

		_physicsSystem = std::make_unique<PhysicsSystem>();

        _time = std::make_unique<Time>();

        _sceneManager = std::make_unique<SceneManager>();

        _resources = std::make_unique<Resources>();

		_audioSystem = std::make_unique<AudioSystem>();
		GM_ASSERT(_audioSystem->Initialize(), "AudioSystem 초기화에 실패했습니다.");

		_uiManager = std::make_unique<UIManager>();
    }

    void Application::Run()
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
        Rectangle(_backHDC, -1, -1, _width + 1, _height + 1);

        _time->Render(_backHDC);
        _sceneManager->Render(_backHDC);
        debug::DebugRenderer::Render(_backHDC);
		_uiManager->Render(_backHDC);

        BitBlt(_hDC, 0, 0, _width, _height, _backHDC, 0, 0, SRCCOPY);
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
}
