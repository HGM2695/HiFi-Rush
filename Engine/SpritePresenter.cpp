#include "SpritePresenter.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Material.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Sampler.h"
#include "SpriteFrame.h"
#include "Texture.h"

namespace gm
{
	SpritePresenter::SpritePresenter() : _material(std::make_unique<Material>()) {}
	SpritePresenter::~SpritePresenter() = default;

	void SpritePresenter::EnsureDefaultMaterial()
	{
		if (_material->GetPipelineState() == nullptr)
		{
			auto pso = APPLICATION.GetResources().Find<PipelineState>(BuiltinResourceKey::SpriteTexturePSO);
			GM_ASSERT_RETURN(pso, "%ls가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::SpriteTexturePSO);
			_material->SetPipelineState(pso);
		}

		if (_material->GetSampler(MaterialSlot::BaseColor) == nullptr)
		{
			auto sampler = APPLICATION.GetResources().Find<Sampler>(BuiltinResourceKey::PointSampler);
			GM_ASSERT_RETURN(sampler, "%ls가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::PointSampler);
			_material->SetSampler(MaterialSlot::BaseColor, sampler);
		}
	}

	void SpritePresenter::Submit(const Matrix& world) const
	{
		if (_material->GetPipelineState() == nullptr
			|| _material->GetTexture(MaterialSlot::BaseColor) == nullptr
			|| _material->GetSampler(MaterialSlot::BaseColor) == nullptr)
			return;

		SpriteRenderItem item{};
		item.world = world;
		item.material = _material.get();
		item.uvRect = CreateUVRect();

		APPLICATION.GetRenderer().SubmitSprite(item);
	}

	void SpritePresenter::SetTexture(const std::shared_ptr<Texture>& texture, MaterialSlot slot)
	{
		GM_ASSERT_RETURN(texture, "texture가 nullptr입니다.");
		_material->SetTexture(slot, texture);
	}

	void SpritePresenter::SetSampler(const std::shared_ptr<Sampler>& sampler, MaterialSlot slot)
	{
		GM_ASSERT_RETURN(sampler, "sampler가 nullptr입니다.");
		_material->SetSampler(slot, sampler);
	}

	void SpritePresenter::SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState)
	{
		GM_ASSERT_RETURN(pipelineState, "pipelineState가 nullptr입니다.");
		_material->SetPipelineState(pipelineState);
	}

	void SpritePresenter::SetMaterial(const MaterialDesc& desc)
	{
		_material = std::make_unique<Material>(desc);
	}

	void SpritePresenter::SetMaterial(const Material& material)
	{
		_material = std::make_unique<Material>(material);
	}

	void SpritePresenter::SetSourceRect(const Rect& rect)
	{
		_sourceRect = rect;
		_useSourceRect = true;
	}

	void SpritePresenter::SetSourceRect(const SpriteFrame& frame)
	{
		SetSourceRect(Rect
		{
			static_cast<float>(frame.Left()),
			static_cast<float>(frame.Top()),
			static_cast<float>(frame.Width()),
			static_cast<float>(frame.Height())
		});
	}

	void SpritePresenter::DisableSourceRect()
	{
		_useSourceRect = false;
	}

	std::shared_ptr<Texture> SpritePresenter::GetTexture(MaterialSlot slot) const
	{
		return _material->GetTexture(slot);
	}

	std::shared_ptr<Sampler> SpritePresenter::GetSampler(MaterialSlot slot) const
	{
		return _material->GetSampler(slot);
	}

	std::shared_ptr<PipelineState> SpritePresenter::GetPipelineState() const
	{
		return _material->GetPipelineState();
	}

	Rect SpritePresenter::CreateUVRect() const
	{
		if (_useSourceRect == false)
			return Rect{ 0.f, 0.f, 1.f, 1.f };

		const std::shared_ptr<Texture> texture = _material->GetTexture(MaterialSlot::BaseColor);
		if (texture == nullptr || texture->GetWidth() == 0 || texture->GetHeight() == 0)
			return Rect{ 0.f, 0.f, 1.f, 1.f };

		return Rect
		{
			_sourceRect.left / static_cast<float>(texture->GetWidth()),
			_sourceRect.top / static_cast<float>(texture->GetHeight()),
			_sourceRect.width / static_cast<float>(texture->GetWidth()),
			_sourceRect.height / static_cast<float>(texture->GetHeight())
		};
	}
}
