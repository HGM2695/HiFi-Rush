#include "Application.h"
#include "windows.h"
#include "Input.h"
#include "Time.h"

namespace gm
{
    Application::Application() = default;
    Application::~Application() = default;

    void Application::Initialize(HWND hWnd, uint32_t width, uint32_t height)
    {
        _hWnd = hWnd;
        _hDC = GetDC(_hWnd);

        _width = width;
        _height = height;

        _input = std::make_unique<Input>();
        _input->Initialize();
        _time = std::make_unique<Time>();
        _time->Initialize();
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
    }

    void Application::LateUpdate()
    {
    }

    void Application::Render()
    {
        _time->Render(_hDC);
    }
}