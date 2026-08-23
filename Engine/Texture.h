#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"
#include "Resource.h"

namespace gm
{
	struct TextureLoadDesc
	{
		std::wstring		path;
		TextureColorSpace	colorSpace = TextureColorSpace::Linear;
	};

	struct TextureDesc
	{
		uint32				width = 0;
		uint32				height = 0;
		uint32				arraySize = 1;
		TextureFormat		format = TextureFormat::RGBA8UNorm;
		TextureBindUsage	bindUsage = TextureBindUsage::ShaderResource;
		const void*			initialData = nullptr;
		uint32				initialDataRowPitch = 0;
	};

	class Texture : public Resource
	{
	public:
		virtual ~Texture() = default;

		static constexpr ResourceType Type = ResourceType::Texture;
		virtual	ResourceType	GetType() const override { return Type; }

		uint32				GetWidth() const { return _width; }
		uint32				GetHeight() const { return _height; }
		uint32				GetArraySize() const { return _arraySize; }
		TextureFormat		GetFormat() const { return _format; }
		TextureBindUsage	GetBindUsage() const { return _bindUsage; }

	protected:
		Texture(uint32 width, uint32 height, uint32 arraySize, TextureFormat format, TextureBindUsage bindUsage) : _width(width), _height(height), _arraySize(arraySize), _format(format), _bindUsage(bindUsage) {}

	private:
		uint32				_width = 0;
		uint32				_height = 0;
		uint32				_arraySize = 1;
		TextureFormat		_format = TextureFormat::Unknown;
		TextureBindUsage	_bindUsage = TextureBindUsage::None;
	};
}
