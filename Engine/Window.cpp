#include "Window.h"
#include "WindowsCore.h"

namespace gm
{
	bool Window::Initialize(const WindowDesc& desc)
	{
        if (RegistClass(desc.instance, desc.className) == false)
            return false;

        if (CreateWindowHandle(desc.width, desc.height, desc.instance, desc.className, desc.title, desc.showCommand) == false)
            return false;

        instance = desc.instance;
        _width = desc.width;
        _height = desc.height;

		return true;
	}

    LRESULT Window::MsgProc(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    LRESULT Window::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Window* window = nullptr;

        if (msg == WM_NCCREATE)
        {
            const auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = static_cast<Window*>(createStruct->lpCreateParams);

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            window->_hWnd = hWnd;
        }
        else
        {
            window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (window)
            return window->MsgProc(hWnd, msg, wParam, lParam);

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    bool Window::RegistClass(HINSTANCE instance, const std::wstring& className)
    {
        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = Window::StaticWndProc;
        wcex.hInstance = instance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = className.c_str();

        const ATOM atom = RegisterClassExW(&wcex);
        GM_ASSERT_RETURN_VAL(atom, false, "윈도우 클래스 등록 실패 error = %lu", GetLastError());

        return true;
    }

    bool Window::CreateWindowHandle(uint32 width, uint32 height, HINSTANCE instance, const std::wstring& className, const std::wstring& title, int nCmdShow)
    {
        RECT rc{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        const int winW = rc.right - rc.left;
        const int winH = rc.bottom - rc.top;

        HWND hWnd = CreateWindowW(
            className.c_str(),
            title.c_str(), 
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 
            0, 
            winW, 
            winH,
            nullptr, 
            nullptr, 
            instance, 
            this
        );

        GM_ASSERT_RETURN_VAL(hWnd, false, "Window 생성에 실패했습니다.");

        ShowWindow(_hWnd, nCmdShow);
        UpdateWindow(_hWnd);

        return true;
    }
}