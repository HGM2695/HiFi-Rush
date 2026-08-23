#include "SkySphereRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "StaticMesh.h"
#include "Texture.h"

namespace gm
{
	namespace
	{
		struct SkySphereCameraConstantVS
		{
			Matrix view;
			Matrix projection;
			Vector3 cameraPosition{};
			float padding = 0.f;
		};
	}

	SkySphereRenderPass::SkySphereRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _commandContext(commandContext), _constantBufferPool(resourceFactory)
	{
		_vertexShader = resources.Find<Shader>(BuiltinResourceKey::SkySphereVS);
		_pixelShader = resources.Find<Shader>(BuiltinResourceKey::SkySpherePS);
	}

	SkySphereRenderPass::~SkySphereRenderPass() = default;

	bool SkySphereRenderPass::Initialize()
	{
		GM_ASSERT_RETURN_VAL(_vertexShader, false, "Sky Sphere Vertex Shader를 찾지 못했습니다.");
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Sky Sphere Pixel Shader를 찾지 못했습니다.");
		return true;
	}

	void SkySphereRenderPass::SetSkySphere(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Texture>& texture)
	{
		_staticMesh = staticMesh;
		_texture = texture;
	}

	void SkySphereRenderPass::ClearSkySphere()
	{
		_staticMesh.reset();
		_texture.reset();
	}

	void SkySphereRenderPass::Render(const CameraViewInfo& viewInfo)
	{
		if (_staticMesh == nullptr || _texture == nullptr)
			return;

		const std::shared_ptr<Mesh>& mesh = _staticMesh->GetMesh();
		if (mesh == nullptr)
			return;

		_constantBufferPool.ResetUsage();
		SkySphereCameraConstantVS cameraConstant{};
		cameraConstant.view = viewInfo.view;
		cameraConstant.projection = viewInfo.projection;
		cameraConstant.cameraPosition = viewInfo.position;
		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(SkySphereCameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstant, sizeof(SkySphereCameraConstantVS));

		RasterizerDesc rasterizerDesc{};
		rasterizerDesc.cullMode = CullMode::None;
		DepthStencilDesc depthStencilDesc{};
		depthStencilDesc.depthEnable = false;
		depthStencilDesc.depthWriteEnable = false;
		BlendDesc blendDesc{};
		SamplerDesc samplerDesc{};
		samplerDesc.filter = TextureFilter::Linear;
		samplerDesc.addressU = TextureAddressMode::Wrap;
		samplerDesc.addressV = TextureAddressMode::Wrap;
		samplerDesc.addressW = TextureAddressMode::Wrap;

		_commandContext.BindMesh(*mesh);
		_commandContext.BindPrimitiveTopology(PrimitiveTopology::TriangleList);
		_commandContext.BindRasterizerState(rasterizerDesc);
		_commandContext.BindDepthStencilState(depthStencilDesc);
		_commandContext.BindBlendState(blendDesc);
		_commandContext.BindVertexShader(*_vertexShader);
		_commandContext.BindPixelShader(*_pixelShader);
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, cameraBuffer);
		_commandContext.BindShaderTexture(0, _texture.get());
		_commandContext.BindSampler(0, &samplerDesc);

		for (const MeshSection& section : _staticMesh->GetSections())
		{
			if (section.indexCount > 0)
				_commandContext.DrawIndexed(section.indexCount, section.indexStart, 0);
		}

		_commandContext.UnbindShaderTextures(0, 1);
	}
}
