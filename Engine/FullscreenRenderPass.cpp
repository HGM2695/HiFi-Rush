#include "FullscreenRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "IGraphicsCommandContext.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"

namespace gm
{
	FullscreenRenderPass::FullscreenRenderPass(Resources& resources, IGraphicsCommandContext& commandContext)
		: _resources(resources), _commandContext(commandContext)
	{
	}

	bool FullscreenRenderPass::Initialize()
	{
		_fullScreenMesh = _resources.Find<Mesh>(BuiltinResourceKey::FullScreenMesh);
		_vertexShader = _resources.Find<Shader>(BuiltinResourceKey::FullScreenTextureVS);
		_copyShader = _resources.Find<Shader>(BuiltinResourceKey::FullScreenTexturePS);
		GM_ASSERT_RETURN_VAL(_fullScreenMesh && _vertexShader && _copyShader, false, "Fullscreen Resource가 로드되지 않았습니다.");
		return true;
	}

	void FullscreenRenderPass::Copy(const Texture& sourceTexture)
	{
		Draw(sourceTexture, *_copyShader);
	}

	void FullscreenRenderPass::Draw(const Texture& sourceTexture, const Shader& pixelShader)
	{
		_commandContext.BindShaderTexture(0, &sourceTexture);
		_commandContext.BindSampler(0, &_samplerDesc);
		Draw(pixelShader);
		_commandContext.UnbindShaderTextures(0, 1);
	}

	void FullscreenRenderPass::Draw(const Shader& pixelShader)
	{
		Draw(pixelShader, BlendDesc{});
	}

	void FullscreenRenderPass::Draw(const Shader& pixelShader, const BlendDesc& blendDesc)
	{
		RasterizerDesc rasterizerDesc{};
		rasterizerDesc.cullMode = CullMode::None;
		DepthStencilDesc depthStencilDesc{};
		depthStencilDesc.depthEnable = false;
		depthStencilDesc.depthWriteEnable = false;
		_commandContext.BindVertexShader(*_vertexShader);
		_commandContext.BindPixelShader(pixelShader);
		_commandContext.BindPrimitiveTopology(PrimitiveTopology::TriangleList);
		_commandContext.BindRasterizerState(rasterizerDesc);
		_commandContext.BindDepthStencilState(depthStencilDesc);
		_commandContext.BindBlendState(blendDesc);
		_commandContext.BindMesh(*_fullScreenMesh);
		_commandContext.DrawIndexed(_fullScreenMesh->GetIndexCount());
	}
}
