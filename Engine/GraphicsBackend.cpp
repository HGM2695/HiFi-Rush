#include "GraphicsBackend.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11GraphicsResourceFactory.h"
#include "D3D11GraphicsCommandContext.h"
#include "D3D11TextRenderer.h"
#include <d3d11.h>

#if GM_ENABLE_DEBUG_TOOLS
#include "D3D11DebugRenderer.h"
#endif

namespace gm
{
	namespace
	{
		GraphicsBackend CreateD3D11Backend(const GraphicsBackendDesc& desc)
		{
			D3D11GraphicsDeviceDesc deviceDesc{};
			deviceDesc.width = desc.width;
			deviceDesc.height = desc.height;
			deviceDesc.hWnd = desc.hWnd;
			deviceDesc.isVSync = desc.isVSync;

			std::unique_ptr<IGraphicsDevice> device = D3D11GraphicsDevice::Create(deviceDesc);
			GM_ASSERT_RETURN_VAL(device, GraphicsBackend{}, "D3D11 그래픽 디바이스 생성에 실패했습니다.");

			D3D11GraphicsDevice* d3d11Device = static_cast<D3D11GraphicsDevice*>(device.get());

			GraphicsBackend backend{};
			backend.resourceFactory = std::make_unique<D3D11GraphicsResourceFactory>(*d3d11Device);

			backend.commandContext = std::make_unique<D3D11GraphicsCommandContext>(d3d11Device->GetImmediateContext());

			backend.textRenderer = std::make_unique<D3D11TextRenderer>();
			GM_ASSERT_RETURN_VAL(backend.textRenderer->Initialize(*device), GraphicsBackend{}, "D3D11 TextRenderer 초기화에 실패했습니다.");

#if GM_ENABLE_DEBUG_TOOLS
			backend.debugRenderer = std::make_unique<D3D11DebugRenderer>(static_cast<D3D11TextRenderer*>(backend.textRenderer.get()));
			GM_ASSERT_RETURN_VAL(backend.debugRenderer->Initialize(*device), GraphicsBackend{}, "D3D11 DebugRenderer 초기화에 실패했습니다.");
#endif

			backend.device = std::move(device);

			return backend;
		}
	}

	GraphicsBackend CreateGraphicsBackend(const GraphicsBackendDesc& desc)
	{
		switch (desc.graphicsAPI)
		{
		case GraphicsAPI::D3D11:
			return CreateD3D11Backend(desc);
		default:
			GM_ASSERT_RETURN_VAL(false, GraphicsBackend{}, "지원하지 않는 그래픽스 API입니다.");
		}
	}
}
