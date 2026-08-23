#pragma once

#include "Component.h"
#include "SpritePresenter.h"

namespace gm
{
	class TransformComponent;
	class Material;
	class Texture;

	class SpriteComponent : public Component
	{
	public:
		SpriteComponent();
		virtual ~SpriteComponent();

		void						SetMaterial(const Material& material) { _presenter.SetMaterial(material); }
		void						SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot = TextureSlot::BaseColor);
		void						SetTexture(const std::wstring& textureKey, TextureSlot slot = TextureSlot::BaseColor);
		void						SetSourceRect(const Rect& rect);
		void						SetSourceRect(const SpriteFrame& frame);
		void						DisableSourceRect();
		void						SetFacingMode(SpriteFacingMode facingMode) { _presenter.SetFacingMode(facingMode); }
		void						SetOpacity(float opacity) { _presenter.SetOpacity(opacity); }

	protected:
		SpritePresenter&			GetPresenter() { return _presenter; }
		const SpritePresenter&		GetPresenter() const { return _presenter; }

		virtual void				OnInitialize() override;
		virtual void				OnRender() override;

	private:
		SpritePresenter				_presenter;
		TransformComponent*			_ownerTransform = nullptr;
	};
}
