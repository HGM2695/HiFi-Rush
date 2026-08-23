#pragma once

#include "GBufferTypes.h"
#include "Texture.h"
#include <array>
#include <memory>
#include <vector>

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;

	class GBuffer
	{
	public:
		bool Initialize(IGraphicsResourceFactory& resourceFactory, uint32 width, uint32 height);
		void Clear(IGraphicsCommandContext& commandContext, const Color& baseColor);

		Texture&							GetTarget(GBufferTarget target) { return *_targets[ToGBufferTargetIndex(target)]; }
		const Texture&						GetTarget(GBufferTarget target) const { return *_targets[ToGBufferTargetIndex(target)]; }
		const std::vector<const Texture*>&	GetRenderTargets() const { return _renderTargets; }

	private:
		std::array<std::unique_ptr<Texture>, GBufferTargetCount>	_targets;
		std::vector<const Texture*>									_renderTargets;
	};
}
