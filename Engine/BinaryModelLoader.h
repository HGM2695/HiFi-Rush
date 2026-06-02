#pragma once

#include "ModelTypes.h"

//#include <istream>
#include <string>

class istream;

namespace gm
{
	class BinaryModelLoader
	{
	public:
		ModelData Load(const std::wstring& filepath);

	private:
		bool ReadBones(std::istream& inputStream, ModelData& modelData);
		bool ReadAnimations(std::istream& inputStream, ModelData& modelData);
		bool ReadMesh(std::istream& inputStream, ModelData& modelData);
		bool ReadMeshes(std::istream& inputStream, ModelData& modelData);
		bool ReadMaterials(std::istream& inputStream, ModelData& modelData);
	};
}
