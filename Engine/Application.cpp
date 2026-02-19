#include "Application.h"
#include "windows.h"
#include "Input.h"
#include "Time.h"
#include "SceneManager.h"

namespace gm
{
    Application::Application() = default;
    Application::~Application() = default;

    void Application::Initialize(HWND hWnd, uint32_t width, uint32_t height)
    {
        initializeWindow(hWnd, width, height);
        createBackDC();
        initializeSubSystem();
    }

    void Application::initializeWindow(HWND hWnd, uint32_t width, uint32_t height)
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
        _input = std::make_unique<Input>();
        _input->Initialize();

        _time = std::make_unique<Time>();
        _time->Initialize();

        _sceneManager = std::make_unique<SceneManager>();
        _sceneManager->Initialize();
    }

    void Application::Run()
    {
        Update();
        LateUpdate();
        Render();
    }

    void Application::Update()
    {
        _input->Update();
        _time->Update();
        _sceneManager->Update();
    }

    void Application::LateUpdate()
    {
        _sceneManager->LateUpdate();
    }

    void Application::Render()
    {
        // Clear Back Buffer
        Rectangle(_backHDC, -1, -1, _width + 1, _height + 1);

        _time->Render(_backHDC);
        _sceneManager->Render(_hDC);

        // Copy BackBuffer to Front Buffer
        BitBlt(_hDC, 0, 0, _width, _height, _backHDC, 0, 0, SRCCOPY);
    }
}