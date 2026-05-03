#include "WindowsCore.h"
#include "LoadScenes.h"
#include "LoadResources.h"

#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

ULONG_PTR gpToken;
Gdiplus::GdiplusStartupInput gpsi;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc();
#endif

    gm::ApplicationDesc desc;
    desc.instance = hInstance;
    desc.title = L"HiFi Rush";
    desc.className = L"HiFiRushWindowClass";
    desc.width = 1600;
    desc.height = 900;
    desc.showCommand = nCmdShow;
    desc.isVSync = true;
    GM_ASSERT_RETURN_VAL(APPLICATION.Initialize(desc), -1, "Application 초기화 실패");

    Gdiplus::GdiplusStartup(&gpToken, &gpsi, nullptr);
    gm::LoadResources();
    gm::SetupScenes();

    APPLICATION.Run();

    Gdiplus::GdiplusShutdown(gpToken);

    return 0;
}