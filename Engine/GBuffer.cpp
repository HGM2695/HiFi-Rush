#include "GBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"

namespace gm
{
	namespace
	{
		constexpr std::array<TextureFormat, GBufferTargetCount> GBufferTargetFormats
		{
			TextureFormat::RGBA8UNorm,
			TextureFormat::RGBA16Float,
			TextureFormat::RGBA8UNorm,
			TextureFormat::RGBA16Float
		};
	}

	bool GBuffer::Initialize(IGraphicsResourceFactory& resourceFactory, uint32 width, uint32 height)
	{
		TextureDesc desc{};
		desc.width = width;
		desc.height = height;
		desc.bindUsage = TextureBindUsage::RenderTarget | TextureBindUsage::ShaderResource;

		_renderTargets.clear();
		_renderTargets.reserve(GBufferTargetCount);
		for (uint32 targetIndex = 0; targetIndex < GBufferTargetCount; ++targetIndex)
		{
			desc.format = GBufferTargetFormats[targetIndex];
			_targets[targetIndex] = resourceFactory.CreateTexture(desc);
			GM_ASSERT_RETURN_VAL(_targets[targetIndex], false, "G-Buffer Target 생성에 실패했습니다. target=%u", targetIndex);
			_renderTargets.push_back(_targets[targetIndex].get());
		}

		return true;
	}

	void GBuffer::Clear(IGraphicsCommandContext& commandContext, const Color& baseColor)
	{
		commandContext.ClearRenderTarget(GetTarget(GBufferTarget::BaseColor), baseColor);
		commandContext.ClearRenderTarget(GetTarget(GBufferTarget::WorldNormal), Colors::Transparent);
		commandContext.ClearRenderTarget(GetTarget(GBufferTarget::MaterialData), Colors::Transparent);
		commandContext.ClearRenderTarget(GetTarget(GBufferTarget::Emissive), Colors::Transparent);
	}
}
