#pragma once

namespace gm
{
	struct Color
	{
		unsigned char r = 255;
		unsigned char g = 255;
		unsigned char b = 255;
		unsigned char a = 255;

		static constexpr Color White() { return { 255, 255, 255, 255 }; }
		static constexpr Color Black() { return { 0, 0, 0, 255 }; }
		static constexpr Color Gray() { return { 128, 128, 128, 255 }; }
		static constexpr Color Red() { return { 255, 0, 0, 255 }; }
		static constexpr Color Green() { return { 0, 255, 0, 255 }; }
		static constexpr Color Blue() { return { 0, 0, 255, 255 }; }
		static constexpr Color Yellow() { return { 255, 255, 0, 255 }; }
		static constexpr Color Cyan() { return { 0, 255, 255, 255 }; }
		static constexpr Color Magenta() { return { 255, 0, 255, 255 }; }
		static constexpr Color Orange() { return { 255, 165, 0, 255 }; }
		static constexpr Color Purple() { return { 128, 0, 128, 255 }; }
		static constexpr Color Transparent() { return { 0, 0, 0, 0 }; }
	};
}
