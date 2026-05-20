#include "SpriteRenderer.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "GameObject.h"
#include "Material.h"
#include "PipelineState.h"
#include "RenderTypes.h"
#include "Renderer.h"
#include "Resources.h"
#include "Sampler.h"
#include "Texture.h"
#include "Transform.h"

namespace gm
{
	SpriteRenderer::SpriteRenderer() : _material(std::make_unique<Material>()) {}
	SpriteRenderer::~SpriteRenderer() = default;

	void SpriteRenderer::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<Transform>();

		EnsureDefaultMaterial();
	}

	void SpriteRenderer::OnRender()
	{
		if (_material->GetPipelineState() == nullptr || _material->GetTexture(0) == nullptr || _material->GetSampler(0) == nullptr)
			return;

		SpriteRenderItem item{};
		item.world = _ownerTransform->GetWorldMatrix();
		item.material = _material.get();
		item.useSourceRect = _useSourceRect;
		item.sourceFrame = _sourceFrame;

		APPLICATION.GetRenderer().SubmitSprite(item);
	}

	void SpriteRenderer::EnsureDefaultMaterial()
	{
		if (_material->GetPipelineState() == nullptr)
		{
			auto pso = APPLICATION.GetResources().Find<PipelineState>(BuiltinResourceKey::SpriteTexturePSO);
			GM_ASSERT_RETURN(pso, "%ls 가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요", BuiltinResourceKey::SpriteTexturePSO);
			_material->SetPipelineState(pso);
		}

		if (_material->GetSampler(0) == nullptr)
		{
			auto sampler = APPLICATION.GetResources().Find<Sampler>(BuiltinResourceKey::PointSampler);
			GM_ASSERT_RETURN(sampler, "%ls 가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요", BuiltinResourceKey::PointSampler);
			_material->SetSampler(0, sampler);
		}
	}

	void SpriteRenderer::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(texture, "texture가 nullptr 입니다.");
		_material->SetTexture(0, texture);
	}

	void SpriteRenderer::SetMaterial(const MaterialDesc& desc)
	{
		_material = std::make_unique<Material>(desc);
	}

	void SpriteRenderer::SetMaterial(const Material& material)
	{
		_material = std::make_unique<Material>(material);
	}

	void SpriteRenderer::DisableSourceRect()
	{
		_useSourceRect = false;
	}

	void SpriteRenderer::SetSourceRect(const SpriteFrame& frame)
	{
		_sourceFrame = frame;
		_useSourceRect = true;
	}

	std::shared_ptr<Texture> SpriteRenderer::GetTexture(uint32 slot) const
	{
		return _material->GetTexture(slot);
	}
}
