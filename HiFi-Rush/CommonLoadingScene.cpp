#include "CommonLoadingScene.h"
#include "Application.h"
#include "BinaryMapLoader.h"
#include "BinaryModelLoader.h"
#include "BinaryNavigationMeshLoader.h"
#include "BuiltinGraphicsResources.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "ChiAnimationTypes.h"
#include "GameObject.h"
#include "HiFiRushAudio.h"
#include "IGraphicsResourceFactory.h"
#include "ITextRenderer.h"
#include "LoadingScreenWidget.h"
#include "MapResource.h"
#include "MonsterResourceInfo.h"
#include "MonsterTypes.h"
#include "NavigationMesh.h"
#include "PathUtil.h"
#include "Paths.h"
#include "PlayerResources.h"
#include "Resources.h"
#include "SceneManager.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "StaticMesh.h"
#include "SoundWave.h"
#include "Texture.h"
#include "UIManager.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>

namespace gm
{
	void CommonLoadingScene::OnInitialize()
	{
		GameObject* cameraObject = SpawnGameObject<GameObject>();
		CameraComponent* camera = cameraObject->AddComponent<CameraComponent>();
		GetCameraManager()->RegisterCamera(L"LoadingCamera", camera);
	}

	void CommonLoadingScene::OnEnter()
	{
		_pendingSceneName = APPLICATION.GetSceneManager().GetPendingSceneName();
		GM_ASSERT_RETURN(_pendingSceneName.empty() == false, "LoadingScene에 목표 Scene이 지정되지 않았습니다.");

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();
		uiManager.AddUserWidget<LoadingScreenWidget>();

		_sceneLoadData = {};
		_accLoadingTime = 0.f;

		const std::wstring targetSceneName = _pendingSceneName;
		Resources* resources = &APPLICATION.GetResources();
		IGraphicsResourceFactory* resourceFactory = &APPLICATION.GetGraphicsResourceFactory();
		// 워커가 완료될 때까지 Resources는 조회만 하고, 등록은 메인 스레드에서 수행합니다.
		_loadFuture = std::async(std::launch::async, [targetSceneName, resources, resourceFactory]
			{
				return LoadPendingSceneResources(targetSceneName, *resources, *resourceFactory);
			});
		_loadingState = LoadingState::LoadingResources;
	}

	void CommonLoadingScene::OnExit()
	{
		APPLICATION.GetUIManager().ClearViewportWidgets();
	}

	void CommonLoadingScene::OnTick(float deltaTime)
	{
		_accLoadingTime += deltaTime;

		if (_loadingState == LoadingState::LoadingResources)
		{
			if (_loadFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				return;

			_sceneLoadData = _loadFuture.get();
			if (_sceneLoadData.succeeded == false)
			{
				_loadingState = LoadingState::Failed;
				GM_ASSERT_RETURN(false, "Scene 리소스 로딩에 실패했습니다. scene=%ls, reason=%ls", _pendingSceneName.c_str(), _sceneLoadData.errorMessage.c_str());
			}

			if (RegisterLoadedResources() == false)
			{
				_loadingState = LoadingState::Failed;
				return;
			}

			_sceneLoadData = {};
			_loadingState = LoadingState::ReadyToTransition;
		}

		if (_loadingState == LoadingState::ReadyToTransition && _accLoadingTime >= _minimumLoadingTime)
			RequestTargetSceneTransition();
	}

	void CommonLoadingScene::OnRender()
	{
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadPendingSceneResources(const std::wstring& sceneName, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		if (sceneName == L"TutorialScene")
			return LoadTutorialSceneResources(resources, resourceFactory);

		if (sceneName == L"OutsideScene")
			return LoadOutsideSceneResources(resources, resourceFactory);

		if (sceneName == L"QamilScene")
			return LoadQamilSceneResources(resources, resourceFactory);

		SceneLoadData result{};
		result.errorMessage = L"지원하지 않는 Scene 로딩 요청입니다.";
		return result;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadTutorialSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadGameplayUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadMapResources(result, resources, resourceFactory, L"TutorialMap.bin") == false)
			return result;

		if (LoadChiResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"tutorial", L"tutorial.bin") == false)
			return result;

		if (LoadRhythmBGM(result, resources, HiFiRushBGM::Tutorial) == false)
			return result;

		result.succeeded = true;
		return result;
	}

	bool CommonLoadingScene::LoadGameplayUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		constexpr std::array<const wchar_t*, 45> texturePaths =
		{{
			L"UI/Bottom_Rhytm_Bar/T_Rhytm_Meter_Cat_Black.dds",
			L"UI/Bottom_Rhytm_Bar/T_Rhytm_Meter_halftone_bg.dds",
			L"UI/Bottom_Rhytm_Bar/T_Rhytm_Meter_BGline.dds",
			L"UI/Bottom_Rhytm_Bar/T_Rhytm_Meter_BGline_Small.dds",
			L"UI/Bottom_Rhytm_Bar/Left_Half_Circle.dds",
			L"UI/Bottom_Rhytm_Bar/Right_Half_Circle.dds",
			L"UI/HP_Bar/Background.dds",
			L"UI/HP_Bar/Background_Full.dds",
			L"UI/HP_Bar/Shadow.dds",
			L"UI/HP_Bar/T_HealthBar_Bar_9.dds",
			L"UI/HP_Bar/Mana_Bar.dds",
			L"UI/HP_Bar/Mana_Bar_Full.dds",
			L"UI/HP_Bar/Reverve_Full0.dds",
			L"UI/HP_Bar/Reverve_Full1.dds",
			L"UI/HP_Bar/Reverve_Full2.dds",
			L"UI/HP_Bar/Reverve_Full3.dds",
			L"UI/HP_Bar/Reverve_Full4.dds",
			L"UI/HP_Bar/Reverve_Full5.dds",
			L"UI/HP_Bar/Reverve_Full6.dds",
			L"UI/HP_Bar/Reverve_Full7.dds",
			L"UI/HP_Bar/Reverve_Full8.dds",
			L"UI/HP_Bar/AfterImage.dds",
			L"UI/RhytmMeter/C_Gauge.dds",
			L"UI/RhytmMeter/C_Shadow.dds",
			L"UI/RhytmMeter/C_GradeMark.dds",
			L"UI/RhytmMeter/C_BlackKeys.dds",
			L"UI/RhytmMeter/B_Gauge.dds",
			L"UI/RhytmMeter/B_Shadow.dds",
			L"UI/RhytmMeter/B_GradeMark.dds",
			L"UI/RhytmMeter/B_InnerSpike.dds",
			L"UI/RhytmMeter/B_OuterSpike.dds",
			L"UI/RhytmMeter/B_RadialEffect.dds",
			L"UI/RhytmMeter/A_Gauge.dds",
			L"UI/RhytmMeter/A_Shadow.dds",
			L"UI/RhytmMeter/A_GradeMark.dds",
			L"UI/RhytmMeter/A_InnerSpike.dds",
			L"UI/RhytmMeter/A_OuterSpike.dds",
			L"UI/RhytmMeter/A_BigRadialEffect.dds",
			L"UI/RhytmMeter/A_Spark.dds",
			L"UI/RhytmMeter/S_Gauge.dds",
			L"UI/RhytmMeter/S_GradeMark.dds",
			L"UI/RhytmMeter/S_Star.dds",
			L"UI/RhytmMeter/S_Thunder.dds",
			L"UI/Fight/T_VFX_tk_word_FIGHT.dds",
			L"UI/Yeah/Font_Yeah.dds",
		}};

		for (const wchar_t* texturePath : texturePaths)
		{
			const std::wstring textureKey = GetFileNameWithoutExtension(texturePath);
			if (resources.Find<Texture>(textureKey))
				continue;

			TextureDesc desc{};
			desc.path = GetTexturePath(texturePath);

			std::shared_ptr<Texture> texture = resourceFactory.CreateTexture(desc);
			if (texture == nullptr)
			{
				outLoadData.errorMessage = L"Gameplay UI Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadMeshTextures(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		const std::filesystem::path meshTexturePath = GetTexturePath(L"Mesh");
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(meshTexturePath))
		{
			if (entry.is_regular_file() == false)
				continue;

			const std::wstring textureKey = GetFileNameWithoutExtension(entry.path().wstring());
			if (textureKey.empty())
				continue;

			if (resources.Find<Texture>(textureKey))
				continue;

			TextureDesc desc{};
			desc.path = entry.path().wstring();

			std::shared_ptr<Texture> texture = resourceFactory.CreateTexture(desc);
			if (texture == nullptr)
			{
				outLoadData.errorMessage = L"환경 Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadMapResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory, const std::wstring& mapFileName)
	{
		const std::wstring mapKey = GetFileNameWithoutExtension(mapFileName);
		std::shared_ptr<MapResource> mapResource = resources.Find<MapResource>(mapKey);
		if (mapResource == nullptr)
		{
			MapData mapData{};
			if (BinaryMapLoader::Load(GetMapPath(mapFileName), mapData) == false)
			{
				outLoadData.errorMessage = L"맵 데이터 로드에 실패했습니다. key=" + mapKey;
				return false;
			}

			mapResource = MapResource::Create(std::move(mapData));
			if (mapResource == nullptr)
			{
				outLoadData.errorMessage = L"MapResource 생성에 실패했습니다. key=" + mapKey;
				return false;
			}

			outLoadData.resources.push_back({ mapKey, mapResource });
		}

		std::vector<uint32> modelIndices;
		modelIndices.reserve(mapResource->GetData().objects.size());
		for (const EnvironmentObjectData& object : mapResource->GetData().objects)
		{
			if (object.renderType != EnvironmentRenderType::None)
				modelIndices.push_back(object.modelIndex);
			for (const EnvironmentComponentData& component : object.components)
			{
				const BeatStaticMeshCycleComponentData* meshCycle = std::get_if<BeatStaticMeshCycleComponentData>(&component);
				if (meshCycle)
					modelIndices.insert(modelIndices.end(), meshCycle->modelIndices.begin(), meshCycle->modelIndices.end());
			}
		}

		std::sort(modelIndices.begin(), modelIndices.end());
		modelIndices.erase(std::unique(modelIndices.begin(), modelIndices.end()), modelIndices.end());

		BinaryModelLoader loader;
		for (uint32 modelIndex : modelIndices)
		{
			const std::wstring modelKey = L"Environment" + std::to_wstring(modelIndex);
			if (resources.Find<StaticMesh>(modelKey) || resources.Find<SkeletalMesh>(modelKey))
				continue;

			const std::wstring modelPath = GetModelPath(L"Binary/Environment/" + modelKey + L".bin");
			ModelData modelData = loader.Load(modelPath);

			const bool hasValidMeshData =
				modelData.type == ModelType::Static
				? modelData.vertices.empty() == false
				: modelData.skinnedVertices.empty() == false;

			if (hasValidMeshData == false || modelData.indices.empty())
			{
				outLoadData.errorMessage = L"환경 모델 데이터가 유효하지 않습니다. key=" + modelKey;
				return false;
			}

			if (modelData.type == ModelType::Static)
			{
				std::shared_ptr<StaticMesh> staticMesh = StaticMesh::Create(modelData, resourceFactory);
				if (staticMesh == nullptr)
				{
					outLoadData.errorMessage = L"환경 StaticMesh 생성에 실패했습니다. key=" + modelKey;
					return false;
				}

				outLoadData.resources.push_back({ modelKey, std::move(staticMesh) });
				continue;
			}

			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"환경 SkeletalMesh 생성에 실패했습니다. key=" + modelKey;
				return false;
			}

			outLoadData.resources.push_back({ modelKey, std::move(skeletalMesh) });

			for (uint32 animationIndex = 0; animationIndex < modelData.animations.size(); ++animationIndex)
			{
				const SkeletalAnimationClipData& clipData = modelData.animations[animationIndex];
				std::shared_ptr<SkeletalAnimationClip> clip = SkeletalAnimationClip::Create(clipData);
				if (clip == nullptr)
				{
					outLoadData.errorMessage = L"환경 SkeletalAnimationClip 생성에 실패했습니다. model=" + modelKey;
					return false;
				}

				const std::wstring animationKey = modelKey + L"." + clipData.name;
				outLoadData.resources.push_back({ animationKey, clip });

				if (animationIndex == 0)
					outLoadData.resources.push_back({ modelKey + L".DefaultAnimation", std::move(clip) });
			}
		}

		std::array<bool, static_cast<size_t>(MonsterType::Count)> requiredMonsterTypes{};
		for (const MonsterSpawnData& spawnData : mapResource->GetData().monsterSpawnDatas)
		{
			const size_t monsterTypeIndex = static_cast<size_t>(spawnData.type);
			if (monsterTypeIndex >= requiredMonsterTypes.size())
			{
				outLoadData.errorMessage = L"맵에 지원하지 않는 Monster Type이 포함되어 있습니다.";
				return false;
			}

			requiredMonsterTypes[monsterTypeIndex] = true;
		}

		for (size_t monsterTypeIndex = 0; monsterTypeIndex < requiredMonsterTypes.size(); ++monsterTypeIndex)
		{
			if (requiredMonsterTypes[monsterTypeIndex] == false)
				continue;

			if (LoadMonsterResources(outLoadData, resources, resourceFactory, static_cast<MonsterType>(monsterTypeIndex)) == false)
				return false;
		}

		return true;
	}

	bool CommonLoadingScene::LoadChiResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		bool hasAllResources = resources.Find<SkeletalMesh>(ChiSkeletalMeshResourceKey) != nullptr;
		hasAllResources &= resources.Find<SkeletalAnimationClip>(ChiDefaultAnimationResourceKey) != nullptr;
		hasAllResources &= resources.Find<StaticMesh>(ChiGuitarResourceKey) != nullptr;
		for (uint32 animationClipIndex = 0; animationClipIndex < ChiAnimationClipIdCount; ++animationClipIndex)
			hasAllResources &= resources.Find<SkeletalAnimationClip>(GetChiAnimationClipKey(static_cast<ChiAnimationClipId>(animationClipIndex))) != nullptr;

		if (hasAllResources)
			return true;

		BinaryModelLoader loader;
		ModelData modelData = loader.Load(GetModelPath(L"Binary/Characters/Chi.bin"));

		if (resources.Find<SkeletalMesh>(ChiSkeletalMeshResourceKey) == nullptr)
		{
			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"Chi SkeletalMesh 생성에 실패했습니다.";
				return false;
			}

			outLoadData.resources.push_back({ ChiSkeletalMeshResourceKey, std::move(skeletalMesh) });
		}

		if (modelData.animations.size() != ChiAnimationClipIdCount)
		{
			outLoadData.errorMessage = L"Chi 애니메이션 개수가 ChiAnimationClipId와 일치하지 않습니다.";
			return false;
		}

		for (uint32 animationClipIndex = 0; animationClipIndex < modelData.animations.size(); ++animationClipIndex)
		{
			const SkeletalAnimationClipData& clipData = modelData.animations[animationClipIndex];
			const ChiAnimationClipId animationClipId = static_cast<ChiAnimationClipId>(animationClipIndex);
			const std::wstring animationClipKey = GetChiAnimationClipKey(animationClipId);
			std::shared_ptr<SkeletalAnimationClip> clip = resources.Find<SkeletalAnimationClip>(animationClipKey);
			if (clip == nullptr)
			{
				clip = SkeletalAnimationClip::Create(clipData);
				if (clip == nullptr)
				{
					outLoadData.errorMessage = L"Chi SkeletalAnimationClip 생성에 실패했습니다.";
					return false;
				}

				outLoadData.resources.push_back({ animationClipKey, clip });
			}

			if (animationClipId == ChiAnimationClipId::Idle && resources.Find<SkeletalAnimationClip>(ChiDefaultAnimationResourceKey) == nullptr)
				outLoadData.resources.push_back({ ChiDefaultAnimationResourceKey, std::move(clip) });
		}

		if (resources.Find<StaticMesh>(ChiGuitarResourceKey) == nullptr)
		{
			ModelData guitarModelData = loader.Load(GetModelPath(L"Binary/Weapon/Guitar.bin"));
			if (guitarModelData.type != ModelType::Static || guitarModelData.vertices.empty() || guitarModelData.indices.empty())
			{
				outLoadData.errorMessage = L"Chi Guitar 모델 데이터가 유효하지 않습니다.";
				return false;
			}

			std::shared_ptr<StaticMesh> guitarMesh = StaticMesh::Create(guitarModelData, resourceFactory);
			if (guitarMesh == nullptr)
			{
				outLoadData.errorMessage = L"Chi Guitar StaticMesh 생성에 실패했습니다.";
				return false;
			}

			outLoadData.resources.push_back({ ChiGuitarResourceKey, std::move(guitarMesh) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadMonsterResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory, MonsterType monsterType)
	{
		const MonsterResourceInfo* resourceInfo = GetMonsterResourceInfo(monsterType);
		if (resourceInfo == nullptr)
		{
			outLoadData.errorMessage = L"지원하지 않는 Monster Type입니다.";
			return false;
		}

		const std::wstring defaultAnimationKey = GetMonsterDefaultAnimationClipKey(monsterType);
		const bool hasRequiredWeapon = resourceInfo->weaponResourceKey == nullptr || resources.Find<StaticMesh>(resourceInfo->weaponResourceKey) != nullptr;
		if (resources.Find<SkeletalMesh>(resourceInfo->commonResourceKey) && resources.Find<SkeletalAnimationClip>(defaultAnimationKey) && hasRequiredWeapon)
			return true;

		BinaryModelLoader loader;
		const std::wstring modelPath = GetModelPath(L"Binary/Monsters/" + std::wstring(resourceInfo->modelFileName));
		ModelData modelData = loader.Load(modelPath);
		if (modelData.type != ModelType::Skeletal || modelData.skinnedVertices.empty() || modelData.indices.empty() || modelData.animations.empty())
		{
			outLoadData.errorMessage = L"Monster 모델 데이터가 유효하지 않습니다. key=" + std::wstring(resourceInfo->commonResourceKey);
			return false;
		}

		if (resourceInfo->defaultAnimationIndex >= modelData.animations.size())
		{
			outLoadData.errorMessage = L"Monster 기본 Animation Index가 유효하지 않습니다. key=" + std::wstring(resourceInfo->commonResourceKey);
			return false;
		}

		if (resources.Find<SkeletalMesh>(resourceInfo->commonResourceKey) == nullptr)
		{
			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"Monster SkeletalMesh 생성에 실패했습니다. key=" + std::wstring(resourceInfo->commonResourceKey);
				return false;
			}

			outLoadData.resources.push_back({ resourceInfo->commonResourceKey, std::move(skeletalMesh) });
		}

		std::shared_ptr<SkeletalAnimationClip> defaultAnimation;
		for (uint32 animationIndex = 0; animationIndex < modelData.animations.size(); ++animationIndex)
		{
			const std::wstring animationKey = GetMonsterAnimationClipKey(monsterType, animationIndex);
			std::shared_ptr<SkeletalAnimationClip> animation = resources.Find<SkeletalAnimationClip>(animationKey);
			if (animation == nullptr)
			{
				animation = SkeletalAnimationClip::Create(modelData.animations[animationIndex]);
				if (animation == nullptr)
				{
					outLoadData.errorMessage = L"Monster SkeletalAnimationClip 생성에 실패했습니다. key=" + animationKey;
					return false;
				}

				outLoadData.resources.push_back({ animationKey, animation });
			}

			if (animationIndex == resourceInfo->defaultAnimationIndex)
				defaultAnimation = std::move(animation);
		}

		if (resources.Find<SkeletalAnimationClip>(defaultAnimationKey) == nullptr)
			outLoadData.resources.push_back({ defaultAnimationKey, std::move(defaultAnimation) });

		if (resourceInfo->weaponResourceKey != nullptr && resources.Find<StaticMesh>(resourceInfo->weaponResourceKey) == nullptr)
		{
			ModelData weaponModelData = loader.Load(GetModelPath(L"Binary/Weapon/" + std::wstring(resourceInfo->weaponModelFileName)));
			if (weaponModelData.type != ModelType::Static || weaponModelData.vertices.empty() || weaponModelData.indices.empty())
			{
				outLoadData.errorMessage = L"Monster Weapon 모델 데이터가 유효하지 않습니다. key=" + std::wstring(resourceInfo->weaponResourceKey);
				return false;
			}

			std::shared_ptr<StaticMesh> weaponMesh = StaticMesh::Create(weaponModelData, resourceFactory);
			if (weaponMesh == nullptr)
			{
				outLoadData.errorMessage = L"Monster Weapon StaticMesh 생성에 실패했습니다. key=" + std::wstring(resourceInfo->weaponResourceKey);
				return false;
			}

			outLoadData.resources.push_back({ resourceInfo->weaponResourceKey, std::move(weaponMesh) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadNavigationMesh(SceneLoadData& outLoadData, Resources& resources, const std::wstring& key, const std::wstring& fileName)
	{
		if (resources.Find<NavigationMesh>(key))
			return true;

		NavigationMeshData navigationMeshData{};
		if (BinaryNavigationMeshLoader::Load(GetResourcePath(L"NavigationMesh/" + fileName), navigationMeshData) == false)
		{
			outLoadData.errorMessage = key + L" NavigationMesh 데이터 로드에 실패했습니다.";
			return false;
		}

		std::shared_ptr<NavigationMesh> navigationMesh = NavigationMesh::Create(NavigationMeshDesc{ navigationMeshData });
		if (navigationMesh == nullptr)
		{
			outLoadData.errorMessage = key + L" NavigationMesh 생성에 실패했습니다.";
			return false;
		}

		outLoadData.resources.push_back({ key, std::move(navigationMesh) });
		return true;
	}

	bool CommonLoadingScene::LoadRhythmBGM(SceneLoadData& outLoadData, Resources& resources, const RhythmBGMDesc& desc)
	{
		if (resources.Find<SoundWave>(desc.commonResourceKey))
			return true;

		SoundWaveDesc soundDesc{};
		soundDesc.path = GetAudioPath(desc.fileName);

		std::shared_ptr<SoundWave> sound = SoundWave::Create(soundDesc);
		if (sound == nullptr)
		{
			outLoadData.errorMessage = L"BGM SoundWave 생성에 실패했습니다. key=" + std::wstring(desc.commonResourceKey);
			return false;
		}

		outLoadData.resources.push_back({ desc.commonResourceKey, std::move(sound) });
		return true;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadOutsideSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadGameplayUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadMapResources(result, resources, resourceFactory, L"OutsideMap.bin") == false)
			return result;

		if (LoadChiResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"jump_outside", L"jump_outside.bin") == false)
			return result;

		if (LoadRhythmBGM(result, resources, HiFiRushBGM::Outside) == false)
			return result;

		result.succeeded = true;
		return result;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadQamilSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadGameplayUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadMapResources(result, resources, resourceFactory, L"QamilMap.bin") == false)
			return result;

		if (LoadChiResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"qamil", L"qamil.bin") == false)
			return result;

		if (LoadRhythmBGM(result, resources, HiFiRushBGM::Qamil) == false)
			return result;

		result.succeeded = true;
		return result;
	}

	bool CommonLoadingScene::RegisterLoadedResources()
	{
		Resources& resources = APPLICATION.GetResources();
		for (const ResourceLoadData& loadData : _sceneLoadData.resources)
		{
			GM_ASSERT_RETURN_VAL(
				resources.Add(loadData.key, loadData.resource),
				false,
				"로드한 리소스 등록에 실패했습니다. key=%ls",
				loadData.key.c_str());
		}

		return true;
	}

	void CommonLoadingScene::RequestTargetSceneTransition()
	{
		APPLICATION.GetSceneManager().RequestSceneChange(_pendingSceneName);
		_loadingState = LoadingState::TransitionRequested;
	}

	float CommonLoadingScene::GetLoadingProgress() const
	{
		if (_loadingState == LoadingState::ReadyToTransition ||
			_loadingState == LoadingState::TransitionRequested)
			return 1.f;

		return 0.f;
	}
}
