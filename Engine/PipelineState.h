#pragma once
#include "Resource.h"
#include "GraphicsTypes.h"

namespace gm
{
	class Shader;

	struct PipelineStateDesc
	{
		std::shared_ptr<class Shader>   vertexShader;
		std::shared_ptr<class Shader>   pixelShader;

		PrimitiveTopology		        topology = PrimitiveTopology::TriangleList;
		RasterizerDesc			        rasterizerDesc{};
		DepthStencilDesc                depthStencilDesc{};
		BlendDesc                       blendDesc{};
	};

	class PipelineState : public Resource
	{
	public:
		virtual ~PipelineState();

		static inline ResourceType Type = ResourceType::PipelineState;
		virtual ResourceType GetType() const override { return Type; }

		PrimitiveTopology	GetTopology() const { return _topology; }
		Shader*				GetVertexShader() const { return _vertexShader.get(); }
		Shader*				GetPixelShader() const { return _pixelShader.get(); }

	protected:
		PipelineState(const PipelineStateDesc& desc);

	protected:
		std::shared_ptr<Shader> _vertexShader;
		std::shared_ptr<Shader> _pixelShader;
		PrimitiveTopology		_topology = PrimitiveTopology::TriangleList;
	};
}