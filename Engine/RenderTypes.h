#pragma once

#include "MathTypes.h"
#include "SpriteFrame.h"
#include <memory>

namespace gm
{
	class Material;
	class Sampler;
	class Texture;

	struct SpriteRenderItem
	{
		Matrix			world = Matrix::CreateScale(1.f);
		const Material* material = nullptr;
		bool			useSourceRect = false;
		SpriteFrame		sourceFrame{};
	};

	struct ColorQuadRenderItem
	{
		Vector2	screenCenter{};
		Vector2	size{};
		Color	color = Colors::White;
	};

	struct TextureQuadRenderItem
	{
		Vector2						screenCenter{};
		Vector2						size{};
		std::shared_ptr<Texture>	texture = nullptr;
		std::shared_ptr<Sampler>	sampler = nullptr;
		bool						useSourceRect = false;
		SpriteFrame					sourceFrame{};
	};
}
