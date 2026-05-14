#pragma once

#include "EngineCore.h"
#include "Widget.h"
#include <string>

namespace gm
{
	class Texture;

	class Image : public Widget
	{
	public:
		Image();
		Image(const std::wstring& textureName);
		~Image();

		void SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
		std::shared_ptr<Texture> GetTexture() const { return _texture; }
		void SetTextureByName(const std::wstring& textureName);

	protected:
		virtual void OnRender(const Vector2& absolutePosition) override;

	private:
		std::shared_ptr<Texture> _texture = nullptr;
	};
}

