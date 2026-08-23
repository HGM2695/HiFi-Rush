#pragma once

#include "GraphicsTypes.h"
#include <memory>

namespace gm
{
	class IGraphicsCommandContext;
	class Mesh;
	class Resources;
	class Shader;
	class Texture;

	class FullscreenRenderPass
	{
	public:
		FullscreenRenderPass(Resources& resources, IGraphicsCommandContext& commandContext);

		bool Initialize();
		void Copy(const Texture& sourceTexture);
		void Draw(const Shader& pixelShader);
		void Draw(const Shader& pixelShader, const BlendDesc& blendDesc);
		void Draw(const Texture& sourceTexture, const Shader& pixelShader);

	private:
		Resources&					_resources;
		IGraphicsCommandContext&	_commandContext;
		std::shared_ptr<Mesh>		_fullScreenMesh;
		std::shared_ptr<Shader>		_vertexShader;
		std::shared_ptr<Shader>		_copyShader;
		SamplerDesc					_samplerDesc;
	};
}
