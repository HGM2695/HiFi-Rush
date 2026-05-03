#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include "IGraphicsDevice.h"

namespace gm
{
	struct D3D11GraphicsDeviceDesc
	{
		uint32	width = 0;
		uint32	height = 0;
		bool	isVSync = true;
		HWND	hWnd = nullptr;
	};

	using Microsoft::WRL::ComPtr;

	class D3D11GraphicsDevice : public IGraphicsDevice
	{
	public:
		virtual ~D3D11GraphicsDevice() = default;

		static std::unique_ptr<IGraphicsDevice> Create(const D3D11GraphicsDeviceDesc& desc);

		virtual void BeginFrame(float red, float green, float blue, float alpha = 1.f) override;
		virtual void EndFrame() override;

		ID3D11Device*			GetNativeDevice() const { return _device.Get(); }
		ID3D11DeviceContext*	GetImmediateContext() const { return _deviceContext.Get(); }
		IDXGISwapChain*			GetSwapChain() const { return _swapChain.Get(); }
		ID3D11RenderTargetView* GetRenderTargetView() const { return _renderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return _depthStencilView.Get(); }

	protected:
		virtual void OnResize() override;

	private:
		D3D11GraphicsDevice(const D3D11GraphicsDeviceDesc& desc) : IGraphicsDevice(desc.width, desc.height, desc.isVSync), _hWnd(desc.hWnd) { }

		bool Initialize(const D3D11GraphicsDeviceDesc& desc);
		bool createDeviceAndSwapChain();
		bool createBackBufferResources();
		void setViewport() const;
		void releaseBackBufferResources();

	private:
		HWND							_hWnd = nullptr;

		ComPtr<ID3D11Device>			_device;
		ComPtr<ID3D11DeviceContext>		_deviceContext;
		ComPtr<IDXGISwapChain>			_swapChain;
		ComPtr<ID3D11RenderTargetView>	_renderTargetView;
		ComPtr<ID3D11Texture2D>			_depthStencilBuffer;
		ComPtr<ID3D11DepthStencilView>	_depthStencilView;
	};
}
