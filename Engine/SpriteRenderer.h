#pragma once

#include "Component.h"
#include <memory>

namespace Gdiplus
{
	class Image;
}

namespace gm
{
	class Transform;

	class SpriteRenderer : public Component
	{
	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();

		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;
		virtual void	OnLateUpdate() override;
		virtual void	OnRender(HDC hDC) override;

		void			ImageLoad(const std::wstring& path);

	private:
		Transform* _Transform{};

		std::unique_ptr<Gdiplus::Image>	 _Image{};
		uint32_t						 _Width{};
		uint32_t						 _Height{};
	}; 
}