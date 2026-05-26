#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"
#include "Widget.h"
#include <string>

namespace gm
{
	class Material;
	class Texture;

	class Image : public Widget
	{
	public:
		Image();
		Image(const std::wstring& textureName);
		~Image();

		void SetTexture(const std::shared_ptr<Texture>& texture);
		void SetTexture(const std::wstring& textureName);
		std::shared_ptr<Texture> GetTexture() const { return _texture; }

		void SetSamplerDesc(const SamplerDesc& desc);
		const SamplerDesc& GetSamplerDesc() const { return _samplerDesc; }

	protected:
		virtual void OnRender(const WidgetGeometry& geometry) override;

	private:
		void CreateMaterial();
		void UpdateMaterial();

	private:
		std::shared_ptr<Texture>	_texture = nullptr;
		SamplerDesc					_samplerDesc{};
		std::unique_ptr<Material>	_material;
	};
}
