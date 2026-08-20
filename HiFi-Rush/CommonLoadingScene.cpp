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
#include "QamilAnimationTypes.h"
#include "QamilResources.h"
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

		if (LoadDialogResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadTutorialUIResources(result, resources, resourceFactory) == false)
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
		if (LoadRhythmBGM(result, resources, HiFiRushBGM::TutorialRoadUp) == false)
			return result;

		result.succeeded = true;
		return result;
	}

	bool CommonLoadingScene::LoadGameplayUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		constexpr std::array texturePaths =
		{
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
			L"UI/Combo/Humburker.dds",
			L"UI/Combo/GrandSlam.dds",
			L"UI/Combo/BreakDown.dds",
			L"UI/Combo/Tremolo.dds",
			L"UI/Combo/DashAttack.dds",
			L"UI/Combo/AirLaunch.dds",
			L"UI/Combo/RiseUp.dds",
			L"UI/Combo/HammerOn.dds",
			L"UI/Combo/AirCountDown.dds",
			L"UI/BeatHit/T_BH_circle_inside.dds",
			L"UI/BeatHit/T_BH_circle_inside_glow.dds",
			L"UI/BeatHit/T_BH_circle_outside.dds",
			L"UI/BeatHit/T_BH_circle_outside_glow.dds",
			L"UI/BeatHit/T_BH_light.dds",
			L"UI/BeatHit/T_BH_light_small.dds",
			L"UI/BeatHit/Thorn.dds",
			L"UI/Dead/Dead0.dds",
			L"UI/Dead/Dead1.dds",
		};

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

	bool CommonLoadingScene::LoadTutorialUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		constexpr std::array<const wchar_t*, 21> texturePaths =
		{{
			L"UI/WeakAttack/WeakAttack.dds",
			L"UI/StrongAttack/StrongAttack.dds",
			L"UI/Tutorial/MouseButton.dds",
			L"UI/Tutorial/MouseButtonRight.dds",
			L"UI/Tutorial/T_tut_rhythm_bar_timeline.dds",
			L"UI/Tutorial/T_tut_rhythm_bar_timing.dds",
			L"UI/Tutorial/T_tut_rhythm_bg_bottom_anim.dds",
			L"UI/Tutorial/T_tut_rhythm_bg_bottom.dds",
			L"UI/Tutorial/T_tut_rhythm_bg_detail_anim.dds",
			L"UI/Tutorial/T_tut_rhythm_bg_top_anim.dds",
			L"UI/Tutorial/T_tut_rhythm_bg_top.dds",
			L"UI/Tutorial/T_tut_rhythm_circle_bg.dds",
			L"UI/Tutorial/T_tut_rhythm_circle.dds",
			L"UI/Tutorial/T_tut_rhythm_vert_line_bg.dds",
			L"UI/Tutorial/T_tut_rhythm_vert_line.dds",
			L"UI/Tutorial/T_Word_CmnResult_Good.dds",
			L"UI/Tutorial/T_Word_CmnResult_Miss.dds",
			L"UI/Tutorial/T_Word_CmnResult_Perfect.dds",
			L"UI/Tutorial/T_Word_Tut_Result_Good.dds",
			L"UI/Tutorial/T_Word_Tut_Result_Miss.dds",
			L"UI/Tutorial/T_Word_Tut_Result_Perfect.dds",
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
				outLoadData.errorMessage = L"Tutorial UI Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		struct TutorialCueInfo
		{
			const wchar_t* resourceKey = nullptr;
			const wchar_t* fileName = nullptr;
		};

		constexpr std::array<TutorialCueInfo, 6> tutorialCueInfos =
		{{
			{ L"TutorialCue.Count", L"Tutorial/0123Yah.wav" },
			{ L"TutorialCue.Pulse", L"Tutorial/Bim.wav" },
			{ L"TutorialCue.One", L"Tutorial/One.wav" },
			{ L"TutorialCue.Two", L"Tutorial/Two.wav" },
			{ L"TutorialCue.Three", L"Tutorial/Three.wav" },
			{ L"TutorialCue.Go", L"Tutorial/Go.wav" },
		}};

		for (const TutorialCueInfo& info : tutorialCueInfos)
		{
			if (resources.Find<SoundWave>(info.resourceKey))
				continue;

			SoundWaveDesc desc{};
			desc.path = GetAudioPath(info.fileName);
			std::shared_ptr<SoundWave> sound = SoundWave::Create(desc);
			if (sound == nullptr)
			{
				outLoadData.errorMessage = L"Tutorial Cue 생성에 실패했습니다. key=" + std::wstring(info.resourceKey);
				return false;
			}

			outLoadData.resources.push_back({ info.resourceKey, std::move(sound) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadQamilUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		constexpr std::array<const wchar_t*, 21> texturePaths =
		{{
			L"UI/Qamil/Qamil_BG0.dds",
			L"UI/Qamil/Qamil_BG1.dds",
			L"UI/Qamil/Qamil_BG2.dds",
			L"UI/Qamil/Qamil_BG3.dds",
			L"UI/Qamil/Qamil_BrokenBG0.dds",
			L"UI/Qamil/Qamil_BrokenBG1.dds",
			L"UI/Qamil/Qamil_BrokenBG2.dds",
			L"UI/Qamil/Qamil_BrokenBG3.dds",
			L"UI/Qamil/Qamil_DamageHP0.dds",
			L"UI/Qamil/Qamil_DamageHP1.dds",
			L"UI/Qamil/Qamil_DamageHP2.dds",
			L"UI/Qamil/Qamil_DamageHP3.dds",
			L"UI/Qamil/Qamil_Gold0.dds",
			L"UI/Qamil/Qamil_Gold1.dds",
			L"UI/Qamil/Qamil_HP0.dds",
			L"UI/Qamil/Qamil_HP1.dds",
			L"UI/Qamil/Qamil_HP2.dds",
			L"UI/Qamil/Qamil_HP3.dds",
			L"UI/Qamil/Qamil_Name.dds",
			L"UI/Qamil/Qamil_Shadow.dds",
			L"UI/Qamil/Qamil_DamageEffect.dds",
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
				outLoadData.errorMessage = L"Qamil UI Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadDialogResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		constexpr std::array<const wchar_t*, 13> texturePaths =
		{{
			L"UI/Modal/BackGround_Top.dds",
			L"UI/Modal/BackGround_Bottom.dds",
			L"UI/Modal/Window2.dds",
			L"UI/Modal/T_talk_chai_00_base.dds",
			L"UI/Modal/T_talk_chai_pleasure_mouth_00.dds",
			L"UI/Modal/T_talk_chai_pleasure_mouth_01.dds",
			L"UI/Modal/T_talk_chai_pleasure_mouth_02.dds",
			L"UI/Modal/T_talk_speaker_00.dds",
			L"UI/Modal/T_talk_speaker_01.dds",
			L"UI/Modal/T_talk_speaker_02.dds",
			L"UI/Modal/Saver0.dds",
			L"UI/Modal/Saver1.dds",
			L"UI/Modal/Saver2.dds",
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
				outLoadData.errorMessage = L"Dialog Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		struct DialogVoiceInfo
		{
			const wchar_t* resourceKey = nullptr;
			const wchar_t* fileName = nullptr;
		};

		constexpr std::array<DialogVoiceInfo, 19> dialogVoiceInfos =
		{{
			{ L"DialogVoice.Tuto1", L"Dialog/Tuto1.mp3" },
			{ L"DialogVoice.Tuto2", L"Dialog/Tuto2.mp3" },
			{ L"DialogVoice.Tuto3", L"Dialog/Tuto3.mp3" },
			{ L"DialogVoice.Tuto4", L"Dialog/Tuto4.mp3" },
			{ L"DialogVoice.Tuto5", L"Dialog/Tuto5.mp3" },
			{ L"DialogVoice.Tuto6", L"Dialog/Tuto6.mp3" },
			{ L"DialogVoice.Tuto7_OneMore", L"Dialog/Tuto7_OneMore.mp3" },
			{ L"DialogVoice.Tuto7_Perfect", L"Dialog/Tuto7_Perfect.mp3" },
			{ L"DialogVoice.Tuto8", L"Dialog/Tuto8.mp3" },
			{ L"DialogVoice.Tuto9", L"Dialog/Tuto9.mp3" },
			{ L"DialogVoice.StrongTuto0", L"Dialog/StrongTuto0.mp3" },
			{ L"DialogVoice.StrongTuto1", L"Dialog/StrongTuto1.mp3" },
			{ L"DialogVoice.StrongTuto2", L"Dialog/StrongTuto2.mp3" },
			{ L"DialogVoice.StrongTuto3", L"Dialog/StrongTuto3.mp3" },
			{ L"DialogVoice.StrongTuto4", L"Dialog/StrongTuto4.mp3" },
			{ L"DialogVoice.Saver0", L"Dialog/Saver0.mp3" },
			{ L"DialogVoice.Saver1", L"Dialog/Saver1.mp3" },
			{ L"DialogVoice.Shuffle", L"Dialog/Shuffle.mp3" },
			{ L"DialogSFX.Applause", L"Dialog/EV_ev8105_01_CrowdApplause_01_Cutting.wav" },
		}};

		for (const DialogVoiceInfo& info : dialogVoiceInfos)
		{
			if (resources.Find<SoundWave>(info.resourceKey))
				continue;

			SoundWaveDesc desc{};
			desc.path = GetAudioPath(info.fileName);
			std::shared_ptr<SoundWave> voice = SoundWave::Create(desc);
			if (voice == nullptr)
			{
				outLoadData.errorMessage = L"Dialog SoundWave 생성에 실패했습니다. key=" + std::wstring(info.resourceKey);
				return false;
			}

			outLoadData.resources.push_back({ info.resourceKey, std::move(voice) });
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

	bool CommonLoadingScene::LoadQamilResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		bool hasAllResources = resources.Find<SkeletalMesh>(QamilSkeletalMeshResourceKey) != nullptr && resources.Find<SkeletalMesh>(QamilMissileSkeletalMeshResourceKey) != nullptr && resources.Find<SkeletalAnimationClip>(QamilMissileAnimationResourceKey) != nullptr;
		for (uint32 animationIndex = 0; animationIndex < QamilAnimationIdCount; ++animationIndex)
			hasAllResources &= resources.Find<SkeletalAnimationClip>(GetQamilAnimationClipKey(static_cast<QamilAnimationId>(animationIndex))) != nullptr;
		if (hasAllResources)
			return true;

		BinaryModelLoader loader;
		const std::wstring modelPath = GetModelPath(L"Binary/Monsters/" + std::wstring(QamilModelFileName));
		ModelData modelData = loader.Load(modelPath);
		if (modelData.type != ModelType::Skeletal || modelData.skinnedVertices.empty() || modelData.indices.empty())
		{
			outLoadData.errorMessage = L"Qamil 모델 데이터가 유효하지 않습니다.";
			return false;
		}

		if (modelData.animations.size() != QamilAnimationIdCount)
		{
			outLoadData.errorMessage = L"Qamil Animation 개수가 QamilAnimationId와 일치하지 않습니다.";
			return false;
		}

		if (resources.Find<SkeletalMesh>(QamilSkeletalMeshResourceKey) == nullptr)
		{
			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"Qamil SkeletalMesh 생성에 실패했습니다.";
				return false;
			}

			outLoadData.resources.push_back({ QamilSkeletalMeshResourceKey, std::move(skeletalMesh) });
		}

		for (uint32 animationIndex = 0; animationIndex < QamilAnimationIdCount; ++animationIndex)
		{
			const QamilAnimationId animationId = static_cast<QamilAnimationId>(animationIndex);
			const std::wstring animationKey = GetQamilAnimationClipKey(animationId);
			if (resources.Find<SkeletalAnimationClip>(animationKey))
				continue;

			std::shared_ptr<SkeletalAnimationClip> animation = SkeletalAnimationClip::Create(modelData.animations[animationIndex]);
			if (animation == nullptr)
			{
				outLoadData.errorMessage = L"Qamil SkeletalAnimationClip 생성에 실패했습니다. key=" + animationKey;
				return false;
			}

			outLoadData.resources.push_back({ animationKey, std::move(animation) });
		}

		ModelData missileModelData = loader.Load(GetModelPath(L"Binary/Monsters/" + std::wstring(QamilMissileModelFileName)));
		if (missileModelData.type != ModelType::Skeletal || missileModelData.skinnedVertices.empty() || missileModelData.indices.empty() || missileModelData.animations.size() != 1)
		{
			outLoadData.errorMessage = L"Qamil Missile 모델 데이터가 유효하지 않습니다.";
			return false;
		}

		if (resources.Find<SkeletalMesh>(QamilMissileSkeletalMeshResourceKey) == nullptr)
		{
			std::shared_ptr<SkeletalMesh> missileMesh = SkeletalMesh::Create(missileModelData, resourceFactory);
			if (missileMesh == nullptr)
			{
				outLoadData.errorMessage = L"Qamil Missile SkeletalMesh 생성에 실패했습니다.";
				return false;
			}
			outLoadData.resources.push_back({ QamilMissileSkeletalMeshResourceKey, std::move(missileMesh) });
		}

		if (resources.Find<SkeletalAnimationClip>(QamilMissileAnimationResourceKey) == nullptr)
		{
			std::shared_ptr<SkeletalAnimationClip> missileAnimation = SkeletalAnimationClip::Create(missileModelData.animations.front());
			if (missileAnimation == nullptr)
			{
				outLoadData.errorMessage = L"Qamil Missile Animation 생성에 실패했습니다.";
				return false;
			}
			outLoadData.resources.push_back({ QamilMissileAnimationResourceKey, std::move(missileAnimation) });
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

	bool CommonLoadingScene::LoadSoundWave(SceneLoadData& outLoadData, Resources& resources, const std::wstring& key, const std::wstring& fileName)
	{
		if (resources.Find<SoundWave>(key))
			return true;

		SoundWaveDesc soundDesc{};
		soundDesc.path = GetAudioPath(fileName);

		std::shared_ptr<SoundWave> sound = SoundWave::Create(soundDesc);
		if (sound == nullptr)
		{
			outLoadData.errorMessage = L"SoundWave 생성에 실패했습니다. key=" + key;
			return false;
		}

		outLoadData.resources.push_back({ key, std::move(sound) });
		return true;
	}

	bool CommonLoadingScene::LoadRhythmBGM(SceneLoadData& outLoadData, Resources& resources, const RhythmBGMDesc& desc)
	{
		return LoadSoundWave(outLoadData, resources, desc.commonResourceKey, desc.fileName);
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadOutsideSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadGameplayUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadDialogResources(result, resources, resourceFactory) == false)
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

		if (LoadSoundWave(result, resources, HiFiRushSound::OutsideTriggerOpen, HiFiRushSound::OutsideTriggerOpenFileName) == false)
			return result;

		result.succeeded = true;
		return result;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadQamilSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadGameplayUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadDialogResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadQamilUIResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadMapResources(result, resources, resourceFactory, L"QamilMap.bin") == false)
			return result;

		if (LoadQamilResources(result, resources, resourceFactory) == false)
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
