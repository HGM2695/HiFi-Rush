#include "PipelineState.h"

namespace gm
{
	PipelineState::~PipelineState() = default;

	PipelineState::PipelineState(const PipelineStateDesc& desc) 
		: _vertexShader(desc.vertexShader), _pixelShader(desc.pixelShader), _topology(desc.topology) {}
}