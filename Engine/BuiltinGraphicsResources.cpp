#include "BuiltinGraphicsResources.h"
#include "IGraphicsResourceFactory.h"
#include "VertexTypes.h"
#include "Resources.h"
#include "Mesh.h"
#include "Shader.h"
#include "PipelineState.h"

namespace gm
{
	namespace
	{
		bool LoadFullScreenMesh(Resources& resources, IGraphicsResourceFactory& factory);
		bool LoadFullScreenTextureShader(Resources& resources, IGraphicsResourceFactory& factory);
		bool LoadFullScreenPipelineState(Resources& resources, IGraphicsResourceFactory& factory);
	}

	bool BuiltinGraphicsResources::Load(Resources& resources, IGraphicsResourceFactory& factory)
	{
		GM_ASSERT_RETURN_VAL(LoadFullScreenMesh(resources, factory), false, "FullScreenMesh 로드 실패");
		GM_ASSERT_RETURN_VAL(LoadFullScreenTextureShader(resources, factory), false, "FullscreenTextureShader 로드 실패");
		GM_ASSERT_RETURN_VAL(LoadFullScreenPipelineState(resources, factory), false, "FullScreenPipelineState 로드 실패");
		return true;
	}

	namespace
	{
		bool LoadFullScreenMesh(Resources& resources, IGraphicsResourceFactory& factory)
		{
			std::vector<VertexPosTex> vertices(4);

			vertices[0].position = Vector3(-0.5f, 0.5f, 0.0f);
			vertices[0].texcoord = Vector2(0.0f, 0.0f);

			vertices[1].position = Vector3(0.5f, 0.5f, 0.0f);
			vertices[1].texcoord = Vector2(1.0f, 0.0f);

			vertices[2].position = Vector3(0.5f, -0.5f, 0.0f);
			vertices[2].texcoord = Vector2(1.0f, 1.0f);

			vertices[3].position = Vector3(-0.5f, -0.5f, 0.0f);
			vertices[3].texcoord = Vector2(0.0f, 1.0f);

			std::vector<uint32> indices = {
				0, 1, 2,
				0, 2, 3,
			};

			MeshDesc desc{};
			desc.vertexData = vertices.data();
			desc.vertexCount = static_cast<uint32>(vertices.size());
			desc.vertexStride = sizeof(VertexPosTex);
			desc.indexData = indices.data();
			desc.indexCount = static_cast<uint32>(indices.size());

			auto mesh = factory.CreateMesh(desc);
			GM_ASSERT_RETURN_VAL(resources.Add(FullScreenMesh, mesh), false, "FullScreenMesh Add 실패");
			return true;
		}

		bool LoadFullScreenTextureShader(Resources& resources, IGraphicsResourceFactory& factory)
		{
			ShaderDesc vsDesc{};
			vsDesc.filePath = L"../Engine/Resources/Shaders/FullScreenTextureVS.hlsl";
			vsDesc.entryPoint = "main";

			auto vertexShader = factory.CreateVertexShader(vsDesc, VertexPosTex::GetLayout());
			GM_ASSERT_RETURN_VAL(resources.Add(FullScreenTextureVS, vertexShader), false, "FullScreenTextureVS Add 실패");

			ShaderDesc psDesc{};
			psDesc.filePath = L"../Engine/Resources/Shaders/FullScreenTexturePS.hlsl";
			psDesc.entryPoint = "main";

			auto pixelShader = factory.CreatePixelShader(psDesc);
			GM_ASSERT_RETURN_VAL(resources.Add(FullScreenTexturePS, pixelShader), false, "FullScreenTexturePS Add 실패");
			return true;
		}

		bool LoadFullScreenPipelineState(Resources& resources, IGraphicsResourceFactory& factory)
		{
			PipelineStateDesc desc{};
			desc.vertexShader = resources.Find<Shader>(FullScreenTextureVS);
			desc.pixelShader = resources.Find<Shader>(FullScreenTexturePS);

			auto pipelinestate = factory.CraetePipelineState(desc);
			GM_ASSERT_RETURN_VAL(resources.Add(FullScreenPipelineState, pipelinestate), false, "FullScreenPipelineState Add 실패");
			return true;
		}
	}
}
