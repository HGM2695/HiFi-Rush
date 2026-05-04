#pragma once

#include "Resource.h"
#include <string>

namespace gm
{
	enum class ShaderStage
	{
		Vertex,
		Pixel,
		//Geometry,
		//Compute,
		//Hull,
		//Domain
	};

	struct ShaderDesc
	{
		std::wstring	filePath;
		std::string		entryPoint = "main";
		std::string		target;
	};

	class Shader : public Resource
	{
	public:
		virtual ~Shader() = default;

		static constexpr ResourceType Type = ResourceType::Shader;
		virtual ResourceType	GetType() const override { return Type; }

		ShaderStage				GetStage() const { return _stage; }
		const std::wstring&		GetFilePath() const { return _filePath; }

	protected:
		Shader(ShaderStage stage, const ShaderDesc& desc) : _stage(stage), _filePath(desc.filePath) {}

	protected:
		ShaderStage		_stage = ShaderStage::Vertex;
		std::wstring	_filePath;
	};
}
