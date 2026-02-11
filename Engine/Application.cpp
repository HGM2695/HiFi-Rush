#include "Application.h"
#include "windows.h"

namespace gm
{
    void Application::Initialize(HWND hWnd)
    {
        _hWnd = hWnd;
        _hDC = GetDC(_hWnd);
    }

    void Application::Run()
    {
        Update();
        LateUpdate();
        Render();
    }

    void Application::Update()
    {
    }

    void Application::LateUpdate()
    {
    }

    void Application::Render()
    {
        Rectangle(_hDC, 500, 500, 600, 600);
    }
}