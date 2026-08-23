#include "PingPongRenderTarget.h"
#include "IGraphicsResourceFactory.h"

namespace gm
{
	bool PingPongRenderTarget::Initialize(IGraphicsResourceFactory& resourceFactory, const TextureDesc& desc)
	{
		for (std::unique_ptr<Texture>& texture : _textures)
		{
			texture = resourceFactory.CreateTexture(desc);
			GM_ASSERT_RETURN_VAL(texture, false, "Ping-Pong Render Target 생성에 실패했습니다.");
		}

		Reset();
		return true;
	}
}
