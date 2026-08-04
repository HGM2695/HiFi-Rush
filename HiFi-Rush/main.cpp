#include "WindowsCore.h"
#include "Application.h"
#include "GraphicsTypes.h"
#include "HiFiRushGameInstance.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    gm::ApplicationDesc desc;
    desc.instance = hInstance;
    desc.title = L"HiFi Rush";
    desc.className = L"HiFiRushWindowClass";
    desc.width = 1600;
    desc.height = 900;
    desc.showCommand = nCmdShow;
    desc.isVSync = false;
    desc.graphicsAPI = gm::GraphicsAPI::D3D11;
    desc.backBufferColor = gm::Colors::Black;

    GM_ASSERT_RETURN_VAL(APPLICATION.Initialize(desc, std::make_unique<gm::HiFiRushGameInstance>()), -1, "Application 초기화에 실패했습니다.");

    APPLICATION.Run();

    return 0;
}
