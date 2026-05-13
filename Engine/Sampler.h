#pragma once

#include "Resource.h"
#include "GraphicsTypes.h"

namespace gm
{
	struct SamplerDesc
	{
		TextureFilter       filter = TextureFilter::Linear;
		TextureAddressMode  addressU = TextureAddressMode::Clamp;
		TextureAddressMode  addressV = TextureAddressMode::Clamp;
		TextureAddressMode  addressW = TextureAddressMode::Clamp;
	};

	class Sampler : public Resource
	{
	public:
		virtual ~Sampler() = default;

		static constexpr ResourceType Type = ResourceType::Sampler;
		virtual ResourceType	GetType() const override { return Type; }

		const SamplerDesc&		GetDesc() const { return _desc; }

	protected:
		Sampler(const SamplerDesc& desc) : _desc(desc) {}

	private:
		SamplerDesc _desc{};
	};
}
