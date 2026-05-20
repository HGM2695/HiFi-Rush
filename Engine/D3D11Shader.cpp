#include "D3D11Shader.h"

namespace gm
{
	namespace
	{
		const char* GetDefaultTarget(ShaderStage stage);
		Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(ShaderStage stage, const D3D11ShaderDesc& desc);
	}

	/// Vertex Shader
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::shared_ptr<Shader> D3D11VertexShader::Create(const D3D11VertexShaderDesc& desc)
	{
		auto shader = std::shared_ptr<D3D11VertexShader>(new D3D11VertexShader(desc));
		GM_ASSERT_RETURN_VAL(shader->Initialize(desc), nullptr, "D3D11 버텍스 셰이더 생성에 실패했습니다.");

		return shader;
	}

	D3D11VertexShader::D3D11VertexShader(const D3D11VertexShaderDesc& desc) : Shader(ShaderStage::Vertex, desc)
	{
	}

	bool D3D11VertexShader::Initialize(const D3D11VertexShaderDesc& desc)
	{
		_byteCode = CompileShader(_stage, desc);
		GM_ASSERT_RETURN_VAL(_byteCode, false, "버텍스 셰이더 컴파일에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(CreateNativeShader(desc.device), false, "D3D11 버텍스 셰이더 생성에 실패했습니다.");

		if (desc.inputElements.empty())
			return true;

		GM_ASSERT_RETURN_VAL(CreateInputLayout(desc.device, desc.inputElements), false, "입력 레이아웃 생성에 실패했습니다.");
		return true;
	}

	bool D3D11VertexShader::CreateNativeShader(ID3D11Device* device)
	{
		const HRESULT hr = device->CreateVertexShader(
			_byteCode->GetBufferPointer(),
			_byteCode->GetBufferSize(),
			nullptr,
			_vertexShader.GetAddressOf()
		);

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "버텍스 셰이더 생성에 실패했습니다.");
		return true;
	}

	bool D3D11VertexShader::CreateInputLayout(ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements)
	{
		const HRESULT hr = device->CreateInputLayout(
			inputElements.data(),
			static_cast<UINT>(inputElements.size()),
			_byteCode->GetBufferPointer(),
			_byteCode->GetBufferSize(),
			_inputLayout.GetAddressOf()
		);

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "입력 레이아웃 생성에 실패했습니다.");
		return true;
	}

	/// Pixel Shader
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::shared_ptr<Shader> D3D11PixelShader::Create(const D3D11PixelShaderDesc& desc)
	{
		auto shader = std::shared_ptr<D3D11PixelShader>(new D3D11PixelShader(desc));
		GM_ASSERT_RETURN_VAL(shader->Initialize(desc), nullptr, "D3D11 픽셀 셰이더 생성에 실패했습니다.");

		return shader;
	}

	D3D11PixelShader::D3D11PixelShader(const D3D11PixelShaderDesc& desc) : Shader(ShaderStage::Pixel, desc) {}

	bool D3D11PixelShader::Initialize(const D3D11PixelShaderDesc& desc)
	{
		_byteCode = CompileShader(_stage, desc);
		GM_ASSERT_RETURN_VAL(_byteCode, false, "픽셀 셰이더 컴파일에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(CreateNativeShader(desc.device), false, "D3D11 픽셀 셰이더 생성에 실패했습니다.");

		return true;
	}

	bool D3D11PixelShader::CreateNativeShader(ID3D11Device* device)
	{
		const HRESULT hr = device->CreatePixelShader(
			_byteCode->GetBufferPointer(),
			_byteCode->GetBufferSize(),
			nullptr,
			_pixelShader.GetAddressOf()
		);

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "픽셀 셰이더 생성에 실패했습니다.");
		return true;
	}

	/// helper
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	namespace
	{
		const char* GetDefaultTarget(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::Vertex:
				return "vs_5_0";
			case ShaderStage::Pixel:
				return "ps_5_0";
			default:
				return "";
			}
		}

		Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(ShaderStage stage, const D3D11ShaderDesc& desc)
		{
			GM_ASSERT_RETURN_VAL(desc.device, nullptr, "D3D11 디바이스가 유효하지 않습니다.");
			GM_ASSERT_RETURN_VAL(desc.filePath.empty() == false, nullptr, "셰이더 파일 경로가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(desc.entryPoint.empty() == false, nullptr, "셰이더 진입점이 비어 있습니다.");

			UINT compileFlags = desc.compileFlags;
#ifdef _DEBUG
			compileFlags |= D3DCOMPILE_DEBUG;
			compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

			const std::string target = desc.target.empty() ? GetDefaultTarget(stage) : desc.target;
			GM_ASSERT_RETURN_VAL(target.empty() == false, nullptr, "셰이더 타겟이 비어 있습니다.");

			Microsoft::WRL::ComPtr<ID3DBlob> byteCode;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

			const HRESULT hr = D3DCompileFromFile(
				desc.filePath.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				desc.entryPoint.c_str(),
				target.c_str(),
				compileFlags,
				0,
				byteCode.GetAddressOf(),
				errorBlob.GetAddressOf()
			);

			if (FAILED(hr))
			{
				const char* message = errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "알 수 없는 셰이더 컴파일 오류입니다.";
				GM_ASSERT_RETURN_VAL(false, nullptr, "셰이더 컴파일에 실패했습니다. %s", message);
			}

			return byteCode;
		}
	}
}
