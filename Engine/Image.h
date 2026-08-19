#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"
#include "Widget.h"
#include <string>

namespace gm
{
	class Material;
	class Texture;

	enum class ImageFillMode
	{
		Horizontal,
		Radial,

		Count
	};

	class Image : public Widget
	{
	public:
		Image();
		Image(const std::wstring& textureName);
		~Image();

		void						SetTexture(const std::shared_ptr<Texture>& texture);
		void						SetTexture(const std::wstring& textureName);
		std::shared_ptr<Texture>	GetTexture() const { return _texture; }

		void						SetSamplerDesc(const SamplerDesc& desc);
		const SamplerDesc&			GetSamplerDesc() const { return _samplerDesc; }

		void						SetOpacity(float opacity);
		float						GetOpacity() const { return _opacity; }
		void						SetColorBlend(Color color, float ratio);
		void						SetColorBlendRatio(float ratio);
		Color						GetBlendColor() const { return _blendColor; }
		float						GetColorBlendRatio() const { return _blendRatio; }
		void						SetColorChannelMapping(Color redChannelColor, Color greenChannelColor, Color blueChannelColor, float ratio = 1.f);
		void						SetFillRatio(float ratio);
		float						GetFillRatio() const { return _fillRatio; }
		void						SetFillMode(ImageFillMode fillMode);
		ImageFillMode				GetFillMode() const { return _fillMode; }
		void						SetRadialFill(float startAngle, float sweepAngle);

	protected:
		virtual void OnRender(const WidgetGeometry& geometry) override;

	private:
		void CreateMaterial();
		void UpdateMaterial();

	private:
		std::shared_ptr<Texture>	_texture = nullptr;
		SamplerDesc					_samplerDesc{};
		Color						_blendColor = Colors::White;
		float						_blendRatio = 0.f;
		Color						_redChannelColor = Colors::Red;
		Color						_greenChannelColor = Colors::Green;
		Color						_blueChannelColor = Colors::Blue;
		float						_channelColorMappingRatio = 0.f;
		float						_opacity = 1.f;
		float						_fillRatio = 1.f;
		ImageFillMode				_fillMode = ImageFillMode::Horizontal;
		float						_radialStartAngle = 0.f;
		float						_radialSweepAngle = 0.f;
		std::unique_ptr<Material>	_material;
	};
}
