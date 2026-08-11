#include "BuiltinGraphicsResources.h"
#include "IGraphicsResourceFactory.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "VertexTypes.h"

namespace gm
{
	namespace
	{
		bool LoadQuadMesh(const wchar_t* key, float halfWidth, Resources& resources, IGraphicsResourceFactory& factory);
		bool LoadVertexShader(const wchar_t* key, const wchar_t* path, const VertexLayoutDesc& layout, Resources& resources, IGraphicsResourceFactory& factory);
		bool LoadPixelShader(const wchar_t* key, const wchar_t* path, Resources& resources, IGraphicsResourceFactory& factory);
	}

	bool BuiltinGraphicsResources::Load(Resources& resources, IGraphicsResourceFactory& factory)
	{
		using namespace BuiltinResourceKey;

		GM_ASSERT_RETURN_VAL(LoadQuadMesh(UnitQuadMesh, 0.5f, resources, factory), false, "UnitQuadMesh 로드 실패");
		GM_ASSERT_RETURN_VAL(LoadQuadMesh(FullScreenMesh, 1.f, resources, factory), false, "FullScreenMesh 로드 실패");

		LoadVertexShader(QuadVS, L"../Engine/Shaders/QuadVS.hlsl", VertexPosTex::GetLayout(), resources, factory);
		LoadVertexShader(FullScreenTextureVS, L"../Engine/Shaders/FullScreenTextureVS.hlsl", VertexPosTex::GetLayout(), resources, factory);
		LoadVertexShader(StaticMeshVS, L"../Engine/Shaders/StaticMeshVS.hlsl", VertexMesh::GetLayout(), resources, factory);
		LoadVertexShader(StaticMeshInstancedVS, L"../Engine/Shaders/StaticMeshInstancedVS.hlsl", VertexMeshInstanced::GetLayout(), resources, factory);
		LoadVertexShader(SkeletalMeshVS, L"../Engine/Shaders/SkeletalMeshVS.hlsl", VertexAnimationMesh::GetLayout(), resources, factory);

		LoadPixelShader(FullScreenTexturePS, L"../Engine/Shaders/FullScreenTexturePS.hlsl", resources, factory);
		LoadPixelShader(SpriteTexturePS, L"../Engine/Shaders/SpriteTexturePS.hlsl", resources, factory);
		LoadPixelShader(SolidColorPS, L"../Engine/Shaders/SolidColorPS.hlsl", resources, factory);
		LoadPixelShader(StaticMeshPS, L"../Engine/Shaders/StaticMeshPS.hlsl", resources, factory);

		return true;
	}

	namespace
	{
		bool LoadQuadMesh(const wchar_t* key, float halfWidth, Resources& resources, IGraphicsResourceFactory& factory)
		{
			std::vector<VertexPosTex> vertices(4);

			vertices[0].position = Vector3(-halfWidth, halfWidth, 0.0f);
			vertices[0].texcoord = Vector2(0.0f, 0.0f);

			vertices[1].position = Vector3(halfWidth, halfWidth, 0.0f);
			vertices[1].texcoord = Vector2(1.0f, 0.0f);

			vertices[2].position = Vector3(halfWidth, -halfWidth, 0.0f);
			vertices[2].texcoord = Vector2(1.0f, 1.0f);

			vertices[3].position = Vector3(-halfWidth, -halfWidth, 0.0f);
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
			GM_ASSERT_RETURN_VAL(resources.Add(key, mesh), false, "Resources에 %ls Add 실패", key);
			return true;
		}

		bool LoadVertexShader(const wchar_t* key, const wchar_t* path, const VertexLayoutDesc& layout, Resources& resources, IGraphicsResourceFactory& factory)
		{
			ShaderDesc desc{};
			desc.filePath = path;
			desc.entryPoint = "main";

			auto vertexShader = factory.CreateVertexShader(desc, layout);
			GM_ASSERT_RETURN_VAL(resources.Add(key, vertexShader), false, "Resources에 %ls Add 실패", key);
			return true;
		}

		bool LoadPixelShader(const wchar_t* key, const wchar_t* path, Resources& resources, IGraphicsResourceFactory& factory)
		{
			ShaderDesc desc{};
			desc.filePath = path;
			desc.entryPoint = "main";

			auto pixelShader = factory.CreatePixelShader(desc);
			GM_ASSERT_RETURN_VAL(resources.Add(key, pixelShader), false, "Resources에 %ls Add 실패", key);
			return true;
		}
	}
}
