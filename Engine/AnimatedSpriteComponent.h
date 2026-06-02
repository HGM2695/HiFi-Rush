#pragma once

#include "Component.h"
#include "SpriteAnimator.h"
#include "SpritePresenter.h"

namespace gm
{
	class TransformComponent;
	class Material;
	class Texture;

	class AnimatedSpriteComponent : public Component
	{
	public:
		AnimatedSpriteComponent();
		virtual ~AnimatedSpriteComponent();

		void								SetMaterial(const Material& material) { _presenter.SetMaterial(material); }
		void								SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot = TextureSlot::BaseColor);
		void								SetSourceRect(const Rect& rect);
		void								SetSourceRect(const SpriteFrame& frame);
		void								DisableSourceRect();

		SpritePresenter&					GetPresenter() { return _presenter; }
		const SpritePresenter&				GetPresenter() const { return _presenter; }
		SpriteAnimator&						GetAnimator() { return _animator; }
		const SpriteAnimator&				GetAnimator() const { return _animator; }

	protected:
		virtual void						OnInitialize() override;
		virtual void						OnTick(float deltaTime) override;
		virtual void						OnRender() override;

	private:
		TransformComponent*					_ownerTransform = nullptr;

		SpritePresenter						_presenter;
		SpriteAnimator						_animator;
	};
}
