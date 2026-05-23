#pragma once

#include "EngineCore.h"
#include "Widget.h"
#include <string>

namespace gm
{
	class Texture;
	class Sampler;

	class Image : public Widget
	{
	public:
		Image();
		Image(const std::wstring& textureName);
		~Image();

		void SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
		void SetTexture(const std::wstring& textureName);
		std::shared_ptr<Texture> GetTexture() const { return _texture; }

		void SetSampler(const std::shared_ptr<Sampler>& sampler) { _sampler = sampler; }
		void SetSampler(const std::wstring& samplerName);
		std::shared_ptr<Sampler> GetSampler() const { return _sampler; }

protected:
		virtual void OnRender(const WidgetGeometry& geometry) override;

	private:
		std::shared_ptr<Texture> _texture = nullptr;
		std::shared_ptr<Sampler> _sampler = nullptr;
	};
}

