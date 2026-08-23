#pragma once

#include "Texture.h"
#include <array>
#include <memory>

namespace gm
{
	class IGraphicsResourceFactory;

	class PingPongRenderTarget
	{
	public:
		bool			Initialize(IGraphicsResourceFactory& resourceFactory, const TextureDesc& desc);
		void			Reset() { _sourceIndex = 0; }
		void			Swap() { _sourceIndex ^= 1; }
		Texture&		GetSource() { return *_textures[_sourceIndex]; }
		Texture&		GetDestination() { return *_textures[1 - _sourceIndex]; }
		const Texture&	GetSource() const { return *_textures[_sourceIndex]; }
		const Texture&	GetDestination() const { return *_textures[1 - _sourceIndex]; }
		Texture&		GetTargetA() { return *_textures[0]; }
		Texture&		GetTargetB() { return *_textures[1]; }
		const Texture&	GetTargetA() const { return *_textures[0]; }
		const Texture&	GetTargetB() const { return *_textures[1]; }

	private:
		std::array<std::unique_ptr<Texture>, 2>		_textures;
		uint32										_sourceIndex = 0;
	};
}
