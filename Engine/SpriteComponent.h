#pragma once

#include "Component.h"
#include "SpritePresenter.h"

namespace gm
{
	class TransformComponent;
	class Texture;

	class SpriteComponent : public Component
	{
	public:
		SpriteComponent();
		virtual ~SpriteComponent();

		virtual TickGroup GetTickGroup() const override { return TickGroup::RenderSubmit; }

		void						SetTexture(const std::shared_ptr<Texture>& texture, MaterialSlot slot = MaterialSlot::BaseColor);
		void						SetSourceRect(const Rect& rect);
		void						SetSourceRect(const SpriteFrame& frame);
		void						DisableSourceRect();

		SpritePresenter&			GetPresenter() { return _presenter; }
		const SpritePresenter&		GetPresenter() const { return _presenter; }

	protected:
		virtual void				OnInitialize() override;
		virtual void				OnRender() override;

	private:
		SpritePresenter				_presenter;
		TransformComponent*					_ownerTransform = nullptr;
	};
}
