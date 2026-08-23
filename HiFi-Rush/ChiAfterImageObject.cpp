#include "ChiAfterImageObject.h"

#include "Material.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	ChiAfterImageObject::ChiAfterImageObject(const ChiAfterImageObjectDesc& desc)
		: _color(desc.color), _colorBlendRatio(std::clamp(desc.colorBlendRatio, 0.f, 1.f)), _lifetime(desc.lifetime), _fadeOut(desc.fadeOut)
	{
		GM_ASSERT_RETURN(desc.sourceMeshComponent && desc.sourceMeshComponent->GetSkeletalMesh(), "Chi After Image를 생성할 Skeletal Mesh가 없습니다.");
		GM_ASSERT_RETURN(desc.lifetime > 0.f, "Chi After Image Lifetime은 0보다 커야 합니다.");

		GetTransform()->SetWorldMatrix(desc.world);
		_meshComponent = AddComponent<SkeletalMeshComponent>();
		_meshComponent->SetSkeletalMesh(desc.sourceMeshComponent->GetSkeletalMesh());
		_meshComponent->GetPose() = desc.sourceMeshComponent->GetPose();
		_meshComponent->SetCastsShadow(false);
		for (uint32 slotIndex = 0; slotIndex < _meshComponent->GetSkeletalMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			const Material* sourceMaterial = desc.sourceMeshComponent->GetMaterial(slotIndex);
			if (sourceMaterial == nullptr)
				continue;

			Material material = *sourceMaterial;
			material.SetShadingModel(ShadingModel::Unlit);
			material.SetSurfaceMode(SurfaceMode::Transparent);
			material.SetOutlineMode(OutlineMode::Disabled);
			material.SetColorBlend(_color, _colorBlendRatio);
			material.SetColorMultiplier(_color);
			material.SetEmissiveIntensity(desc.emissiveIntensity);
			BlendDesc blendDesc{};
			blendDesc.blendEnable = true;
			blendDesc.srcBlend = BlendFactor::SrcAlpha;
			blendDesc.destBlend = BlendFactor::One;
			material.SetBlendDesc(blendDesc);
			_meshComponent->SetMaterial(slotIndex, material);
		}
	}

	void ChiAfterImageObject::UpdateSnapshot(const SkeletalMeshComponent& sourceMeshComponent, const Matrix& world)
	{
		if (_meshComponent == nullptr)
			return;

		GetTransform()->SetWorldMatrix(world);
		_meshComponent->GetPose() = sourceMeshComponent.GetPose();
		SetRender(true);
	}

	void ChiAfterImageObject::SetColor(const Color& color)
	{
		_color = color;
		UpdateOpacity(1.f);
	}

	void ChiAfterImageObject::SetEmissiveIntensity(float intensity)
	{
		if (_meshComponent == nullptr || _meshComponent->GetSkeletalMesh() == nullptr)
			return;
		for (uint32 slotIndex = 0; slotIndex < _meshComponent->GetSkeletalMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _meshComponent->GetMaterial(slotIndex);
			if (material)
				material->SetEmissiveIntensity(intensity);
		}
	}

	void ChiAfterImageObject::SetColorBlendRatio(float ratio)
	{
		_colorBlendRatio = std::clamp(ratio, 0.f, 1.f);
		UpdateOpacity(1.f);
	}

	void ChiAfterImageObject::Finish()
	{
		Destroy();
	}

	void ChiAfterImageObject::OnTick(float deltaTime)
	{
		_elapsed += std::max(0.f, deltaTime);
		const float ratio = std::clamp(_elapsed / _lifetime, 0.f, 1.f);
		if (_fadeOut)
			UpdateOpacity(1.f - ratio);
		if (ratio >= 1.f)
			Destroy();
	}

	void ChiAfterImageObject::UpdateOpacity(float opacity)
	{
		if (_meshComponent == nullptr || _meshComponent->GetSkeletalMesh() == nullptr)
			return;

		const float alpha = _color.w * std::clamp(opacity, 0.f, 1.f);
		for (uint32 slotIndex = 0; slotIndex < _meshComponent->GetSkeletalMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _meshComponent->GetMaterial(slotIndex);
			if (material)
			{
				material->SetColorBlend(_color, _colorBlendRatio);
				material->SetColorMultiplier(Color{ _color.x, _color.y, _color.z, alpha });
			}
		}
	}
}
