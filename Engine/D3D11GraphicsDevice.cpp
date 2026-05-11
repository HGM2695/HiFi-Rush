#include "D3D11GraphicsDevice.h"
#include "DirectXMath.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace gm
{
	using namespace DirectX;

	std::unique_ptr<IGraphicsDevice> D3D11GraphicsDevice::Create(const D3D11GraphicsDeviceDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.hWnd, nullptr, "윈도우 핸들이 유효하지 않습니다.");
		std::unique_ptr<D3D11GraphicsDevice> device(new D3D11GraphicsDevice(desc));
		GM_ASSERT_RETURN_VAL(device->Initialize(desc), nullptr, "D3D11 디바이스 초기화에 실패했습니다.");

		return device;
	}

	void D3D11GraphicsDevice::BeginFrame(const Color& color)
	{
		_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
		_deviceContext->ClearRenderTargetView(_renderTargetView.Get(), color);
		_deviceContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	void D3D11GraphicsDevice::EndFrame()
	{
		const UINT syncInterval = _isVSync ? 1 : 0;
		const HRESULT hr = _swapChain->Present(syncInterval, 0);
		GM_ASSERT(SUCCEEDED(hr), "스왑체인 Present에 실패했습니다.");
	}

	void D3D11GraphicsDevice::OnResize()
	{
		releaseBackBufferResources();

		const HRESULT hr = _swapChain->ResizeBuffers(0, _width, _height, DXGI_FORMAT_UNKNOWN, 0);
		GM_ASSERT_RETURN(SUCCEEDED(hr), "스왑체인 ResizeBuffers에 실패했습니다.");
		GM_ASSERT_RETURN(createBackBufferResources(), "리사이즈 후 백 버퍼 리소스 생성에 실패했습니다.");

		setViewport();
	}

	bool D3D11GraphicsDevice::Initialize(const D3D11GraphicsDeviceDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(createDeviceAndSwapChain(), false, "DX11 디바이스와 스왑체인 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(createBackBufferResources(), false, "백 버퍼 리소스 생성에 실패했습니다.");
		setViewport();

		return true;
	}

	bool D3D11GraphicsDevice::createDeviceAndSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = _width;
		swapChainDesc.BufferDesc.Height = _height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 3;
		swapChainDesc.OutputWindow = _hWnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		const D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		D3D_FEATURE_LEVEL createdFeatureLevel{};
		const HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels,
			sizeof(featureLevels) / sizeof(featureLevels[0]),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			_swapChain.GetAddressOf(),
			_device.GetAddressOf(),
			&createdFeatureLevel,
			_deviceContext.GetAddressOf()
		);

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D3D11CreateDeviceAndSwapChain에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(
			createdFeatureLevel == D3D_FEATURE_LEVEL_11_0 || createdFeatureLevel == D3D_FEATURE_LEVEL_11_1,
			false,
			"D3D11 Feature Level 11 이상을 지원하지 않습니다."
		);

		return true;
	}

	bool D3D11GraphicsDevice::createBackBufferResources()
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
		HRESULT hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "스왑체인에서 백 버퍼를 가져오지 못했습니다.");

		hr = _device->CreateRenderTargetView(backBuffer.Get(), nullptr, _renderTargetView.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "렌더 타겟 뷰 생성에 실패했습니다.");

		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = _width;
		depthStencilDesc.Height = _height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = _device->CreateTexture2D(&depthStencilDesc, nullptr, _depthStencilBuffer.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "깊이 스텐실 버퍼 생성에 실패했습니다.");

		hr = _device->CreateDepthStencilView(_depthStencilBuffer.Get(), nullptr, _depthStencilView.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "깊이 스텐실 뷰 생성에 실패했습니다.");

		return true;
	}

	void D3D11GraphicsDevice::setViewport() const
	{
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.Width = static_cast<float>(_width);
		viewport.Height = static_cast<float>(_height);
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		_deviceContext->RSSetViewports(1, &viewport);
	}

	void D3D11GraphicsDevice::releaseBackBufferResources()
	{
		if (_deviceContext)
		{
			ID3D11RenderTargetView* nullRTV = nullptr;
			_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		}

		_renderTargetView.Reset();
		_depthStencilView.Reset();
		_depthStencilBuffer.Reset();
	}
}
