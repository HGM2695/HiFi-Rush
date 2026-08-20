#include "BeatTextureSequenceComponent.h"

#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "Material.h"
#include "Resources.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"
#include "Texture.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace gm
{
	BeatTextureSequenceComponent::BeatTextureSequenceComponent(Resources& resources, const BeatSystem& beatSystem, BeatTextureSequenceDesc desc)
		: _resources(resources), _beatSystem(beatSystem), _desc(std::move(desc))
	{}

	void BeatTextureSequenceComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.framesPerBeat > 0.f, "BeatTextureSequenceComponent의 Frames Per Beat는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_desc.initialTextureKeys.empty() == false, "BeatTextureSequenceComponent의 기본 Texture Sequence가 비어 있습니다.");
		GM_ASSERT_RETURN(_desc.triggeredTextureKeys.empty() == false, "BeatTextureSequenceComponent의 Trigger Texture Sequence가 비어 있습니다.");
		GM_ASSERT_RETURN(ResolveMaterial(), "BeatTextureSequenceComponent의 Material을 찾을 수 없습니다.");
		GM_ASSERT_RETURN(ResolveTextures(_desc.initialTextureKeys, _initialTextures), "BeatTextureSequenceComponent의 기본 Texture를 찾을 수 없습니다.");
		GM_ASSERT_RETURN(ResolveTextures(_desc.triggeredTextureKeys, _triggeredTextures), "BeatTextureSequenceComponent의 Trigger Texture를 찾을 수 없습니다.");

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "BeatTextureSequenceComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { ScheduleTriggeredSequence(startBeat); },
			[this]() { ResetSequence(); }), "BeatTextureSequenceComponent의 Trigger Binding에 실패했습니다.");
		ResetSequence();
	}

	void BeatTextureSequenceComponent::OnTick(float)
	{
		if (_material == nullptr)
			return;

		UpdateSequenceState();
		ApplyCurrentFrame();
	}

	bool BeatTextureSequenceComponent::ResolveMaterial()
	{
		StaticMeshComponent* staticMesh = GetOwner().GetComponent<StaticMeshComponent>();
		SkeletalMeshComponent* skeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL((staticMesh != nullptr) != (skeletalMesh != nullptr), false, "BeatTextureSequenceComponent는 하나의 Static 또는 Skeletal Mesh Component가 필요합니다.");
		_material = staticMesh ? staticMesh->GetMaterial(_desc.materialSlot) : skeletalMesh->GetMaterial(_desc.materialSlot);
		return _material != nullptr;
	}

	bool BeatTextureSequenceComponent::ResolveTextures(const std::vector<std::wstring>& textureKeys, std::vector<std::shared_ptr<Texture>>& outTextures) const
	{
		outTextures.clear();
		outTextures.reserve(textureKeys.size());
		for (const std::wstring& textureKey : textureKeys)
		{
			std::shared_ptr<Texture> texture = _resources.Find<Texture>(textureKey);
			GM_ASSERT_RETURN_VAL(texture, false, "Beat Texture Sequence 리소스가 없습니다. key=%ls", textureKey.c_str());
			outTextures.push_back(std::move(texture));
		}
		return true;
	}

	void BeatTextureSequenceComponent::ScheduleTriggeredSequence(float startBeat)
	{
		if (_isTriggered || _triggerBeat.has_value())
			return;
		_triggerBeat = startBeat;
	}

	void BeatTextureSequenceComponent::ResetSequence()
	{
		_isTriggered = false;
		_triggerBeat.reset();
		_currentTexture.reset();
		ApplyCurrentFrame();
	}

	void BeatTextureSequenceComponent::UpdateSequenceState()
	{
		if (_triggerBeat.has_value() == false || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < *_triggerBeat)
			return;
		_isTriggered = true;
		_triggerBeat.reset();
		_currentTexture.reset();
	}

	void BeatTextureSequenceComponent::ApplyCurrentFrame()
	{
		if (_material == nullptr || _initialTextures.empty() || _triggeredTextures.empty())
			return;

		const std::vector<std::shared_ptr<Texture>>& textures = _isTriggered ? _triggeredTextures : _initialTextures;
		const float currentBeat = _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() : 0.f;
		const float sequencePosition = std::max(0.f, currentBeat + _desc.phaseOffsetBeats) * _desc.framesPerBeat;
		const size_t frameIndex = static_cast<size_t>(sequencePosition) % textures.size();
		const std::shared_ptr<Texture>& texture = textures[frameIndex];
		if (_currentTexture == texture)
			return;
		_currentTexture = texture;
		_material->SetTexture(_desc.textureSlot, texture);
	}
}
