#pragma once

#include "Shader.h"
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11ShaderDesc : ShaderDesc
	{
		ID3D11Device*	device = nullptr;
		UINT			compileFlags = 0;
	};

	/// Vertex Shader
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct D3D11VertexShaderDesc : D3D11ShaderDesc
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
	};

	class D3D11VertexShader : public Shader
	{
		friend class D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11VertexShader() = default;

		ID3D11VertexShader* GetNativeShader() const { return _vertexShader.Get(); }
		ID3D11InputLayout*	GetInputLayout() const { return _inputLayout.Get(); }

	private:
		static std::shared_ptr<Shader> Create(const D3D11VertexShaderDesc& desc);

		D3D11VertexShader(const D3D11VertexShaderDesc& desc);

		bool Initialize(const D3D11VertexShaderDesc& desc);
		bool CreateNativeShader(ID3D11Device* device);
		bool CreateInputLayout(ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements);

	private:
		Microsoft::WRL::ComPtr<ID3DBlob>			_byteCode;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	_inputLayout;
	};

	/// Pixel Shader
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct D3D11PixelShaderDesc : D3D11ShaderDesc
	{
	};

	class D3D11PixelShader : public Shader
	{
		friend class D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11PixelShader() = default;

		ID3D11PixelShader*	GetNativeShader() const { return _pixelShader.Get(); }

	private:
		static std::shared_ptr<Shader> Create(const D3D11PixelShaderDesc& desc);

		D3D11PixelShader(const D3D11PixelShaderDesc& desc);

		bool Initialize(const D3D11PixelShaderDesc& desc);
		bool CreateNativeShader(ID3D11Device* device);

	private:
		Microsoft::WRL::ComPtr<ID3DBlob>			_byteCode;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	_pixelShader;
	};
}
