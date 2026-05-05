#pragma once

#include "EngineCore.h"

namespace gm
{
	class IGraphicsDevice;
	class IGraphicsResourceFactory;

	enum class GraphicsAPI
	{
		D3D11,
	};

	struct GraphicsBackend
	{
		std::unique_ptr<IGraphicsDevice> device;
		std::unique_ptr<IGraphicsResourceFactory> resourceFactory;
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
