#pragma once

#include "Resource.h"
#include "ModelTypes.h"
#include <vector>

namespace gm
{
	class IGraphicsResourceFactory;
	class Mesh;

	class StaticMesh : public Resource
	{
	public:
		static std::shared_ptr<StaticMesh> Create(const ModelData& modelData, IGraphicsResourceFactory& resourceFactory);

		static inline ResourceType Type = ResourceType::StaticMesh;
		virtual ResourceType GetType() const override { return Type; }

		const Matrix&							GetPreTransform() const { return _preTransform; }
		const std::shared_ptr<Mesh>&			GetMesh() const { return _mesh; }
		const std::vector<MeshSection>&			GetSections() const { return _sections; }
		const std::vector<MeshTextureSet>&		GetTextureSets() const { return _textureSets; }

		const MeshSection*						GetSection(uint32 index) const;
		const MeshTextureSet*					GetTextureSet(uint32 index) const;
		uint32									GetTextureSetCount() const { return static_cast<uint32>(_textureSets.size()); }

	private:
		StaticMesh(std::shared_ptr<Mesh> mesh, const ModelData& modelData);

	private:
		Matrix							_preTransform = Matrix::CreateScale(1.f);

		std::shared_ptr<Mesh>			_mesh;

		std::vector<MeshSection>		_sections;
		std::vector<MeshTextureSet>		_textureSets;
	};
}
