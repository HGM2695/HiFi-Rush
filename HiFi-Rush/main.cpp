#include "WindowsCore.h"
#include "LoadScenes.h"
#include "LoadResources.h"
#include "LoadDebugTools.h"
#include "GraphicsTypes.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
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
    desc.graphicsAPI = gm::GraphicsAPI::D3D11;

    GM_ASSERT_RETURN_VAL(APPLICATION.Initialize(desc), -1, "Application 초기화 실패");

    gm::LoadResources();
    gm::SetupScenes();
    gm::LoadDebugTools();

    APPLICATION.Run();

    return 0;
}
