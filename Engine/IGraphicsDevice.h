#pragma once

#include "EngineCore.h"

namespace gm
{
	class IGraphicsDevice
	{
	public:
		virtual ~IGraphicsDevice() = default;

		virtual void BeginFrame(const Color& color) = 0;
		virtual void EndFrame() = 0;

		void Resize(uint32 width, uint32 height)
		{
			_width = width;
			_height = height;
			OnResize();
		}

		uint32	GetWidth() const { return _width; }
		uint32	GetHeight() const { return _height; }
		bool	IsVSync() const { return _isVSync; }

	protected:
		IGraphicsDevice(uint32 width, uint32 height, bool isVSync) : _width(width), _height(height), _isVSync(isVSync) {}
		virtual void OnResize() = 0;

	protected:
		uint32	_width = 0;
		uint32	_height = 0;
		bool	_isVSync = true;
	};
}
