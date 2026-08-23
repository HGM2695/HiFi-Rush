#pragma once

#include "IGraphicsDevice.h"
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;

namespace gm
{
	struct D3D11GraphicsDeviceDesc
	{
		uint32	width = 0;
		uint32	height = 0;
		bool	isVSync = true;
		HWND	hWnd = nullptr;
	};

	class D3D11GraphicsDevice : public IGraphicsDevice
	{
	public:
		virtual ~D3D11GraphicsDevice() = default;

		static std::unique_ptr<IGraphicsDevice> Create(const D3D11GraphicsDeviceDesc& desc);

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
		void releaseBackBufferResources();

	private:
		HWND											_hWnd = nullptr;

		Microsoft::WRL::ComPtr<ID3D11Device>			_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		_deviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain>			_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	_depthStencilView;
	};
}
