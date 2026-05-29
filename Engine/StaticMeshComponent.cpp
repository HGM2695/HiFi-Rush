#include "StaticMeshComponent.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "GameObject.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "StaticMesh.h"
#include "TransformComponent.h"

namespace gm
{
	StaticMeshComponent::StaticMeshComponent() = default;
	StaticMeshComponent::~StaticMeshComponent() = default;

	void StaticMeshComponent::SetStaticMesh(const std::shared_ptr<StaticMesh>& staticMesh)
	{
		GM_ASSERT_RETURN(staticMesh, "StaticMeshComponent에 설정할 StaticMesh가 nullptr입니다.");

		_staticMesh = staticMesh;
		_preTransform = _staticMesh->GetPreTransform();

		const uint32 slotCount = _staticMesh->GettextureSlotCount();
		_materials.clear();
		_materials.reserve(slotCount);

		Resources& resources = APPLICATION.GetResources();
		for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
		{
			const MeshTextureSlot* textureSlot = _staticMesh->GettextureSlot(slotIndex);
			if (textureSlot == nullptr)
			{
				_materials.push_back(nullptr);
				continue;
			}

			Material::MaterialBuilder builder(resources);
			builder.SetVertexShader(BuiltinResourceKey::StaticMeshVS)
				.SetPixelShader(BuiltinResourceKey::StaticMeshPS)
				.SetSamplerAddressMode(TextureSlot::BaseColor, TextureAddressMode::Wrap)
				.SetCullMode(CullMode::None);

			for (uint32 textureSlotIndex = 0; textureSlotIndex < TextureSlotCount; ++textureSlotIndex)
			{
				const std::wstring& textureKey = textureSlot->textureKeys[textureSlotIndex];
				if (textureKey.empty())
					continue;

				builder.SetTexture(ToTextureSlot(textureSlotIndex), textureKey);
			}

			_materials.push_back(std::make_unique<Material>(builder.Build()));
		}
	}

	void StaticMeshComponent::SetMaterial(uint32 slotIndex, const Material& material)
	{
		if (slotIndex >= _materials.size())
			_materials.resize(slotIndex + 1);

		_materials[slotIndex] = std::make_unique<Material>(material);
	}

	Material* StaticMeshComponent::GetMaterial(uint32 slotIndex) const
	{
		if (slotIndex >= _materials.size())
			return nullptr;

		return _materials[slotIndex].get();
	}

	void StaticMeshComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<TransformComponent>();
		GM_ASSERT_RETURN(_ownerTransform, "StaticMeshComponent 소유자의 TransformComponent가 존재하지 않습니다.");
	}

	void StaticMeshComponent::OnRender()
	{
		if (_staticMesh == nullptr || _ownerTransform == nullptr)
			return;

		StaticMeshRenderItem item{};
		item.world = _preTransform * _ownerTransform->GetWorldMatrix();
		item.staticMesh = _staticMesh.get();
		item.materials.reserve(_staticMesh->GettextureSlotCount());

		for (uint32 i = 0; i < _staticMesh->GettextureSlotCount(); ++i)
		{
			item.materials.push_back(GetMaterial(i));
		}

		APPLICATION.GetRenderer().SubmitStaticMesh(item);
	}
}
