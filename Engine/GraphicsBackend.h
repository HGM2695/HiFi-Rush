#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

namespace gm
{
	class IGraphicsDevice;
	class IGraphicsResourceFactory;
	class IGraphicsCommandContext;
	class IDebugRenderer;

	struct GraphicsBackend
	{
		std::unique_ptr<IGraphicsDevice>			device;
		std::unique_ptr<IGraphicsResourceFactory>	resourceFactory;
		std::unique_ptr<IGraphicsCommandContext>	commandContext;
		std::unique_ptr<IDebugRenderer>				debugRenderer;
	};

	struct GraphicsBackendDesc
	{
		GraphicsAPI graphicsAPI = GraphicsAPI::D3D11;
		HWND		hWnd = nullptr;
		uint32		width = 0;
		uint32		height = 0;
		bool		isVSync = true;
	};

	GraphicsBackend CreateGraphicsBackend(const GraphicsBackendDesc& desc);
}
