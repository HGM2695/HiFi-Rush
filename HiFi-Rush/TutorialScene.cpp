#include "TutorialScene.h"
#include "AttackGuideWidget.h"
#include "RhythmTutorialWidget.h"
#include "Application.h"
#include "BeatSystem.h"
#include "ChiStateMachineComponent.h"
#include "DialogComponent.h"
#include "Input.h"
#include "Resources.h"
#include "GameObject.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "PhysicsSystem.h"
#include "NavMeshSystem.h"
#include "BoxCollider2DComponent.h"
#include "CameraManager.h"
#include "TransformComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalAnimationClip.h"
#include "NavigationMesh.h"
#include "HiFiRushAudio.h"
#include "HiFiRushDialogData.h"
#include "HiFiRushStatics.h"
#include "PlayerSpawner.h"
#include "PlayerControlComponent.h"
#include "SceneDebugTools.h"
#include "TriggerSequenceSystem.h"
#include "UIManager.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t RoadUpTriggerId[] = L"Tutorial.RoadUp";
	}

	void TutorialScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);
		APPLICATION.GetInput().SetCursorLocked(true);

		std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"tutorial");
		GM_ASSERT_RETURN(navigationMesh, "tutorial NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Tutorial);
		InitializeGameplayUI();
		SetGameplayStatusUIVisible(false);
		InitializeTutorialFlow();
		GM_ASSERT_RETURN(PlayDialogSequence(HiFiRushDialogSequenceIds::TutorialIntro), "Tutorial 시작 Dialog 재생에 실패했습니다.");
	}

	void TutorialScene::OnExit()
	{
		_dialogFinishedConnection.Disconnect();
		_dialogBranchRequestedConnection.Disconnect();
		_attackGuideCompletedConnection.Disconnect();
		_rhythmTutorialInputPhaseConnection.Disconnect();
		_rhythmTutorialCompletedConnection.Disconnect();
		if (_playerControlComponent)
			_playerControlComponent->ReleaseControls(this);
		_attackGuideWidget = nullptr;
		_rhythmTutorialWidget = nullptr;
		_playerControlComponent = nullptr;
		_pendingDialogSequenceId = nullptr;
		_isRoadUpBGMQueued = false;
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void TutorialScene::OnInitialize()
	{
		//InitializeSubObject();
		GM_ASSERT_RETURN(InitializeMap(L"TutorialMap"), "Tutorial Map 구성에 실패했습니다.");

		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ 6.5f, 1.4f, 0.f };
		playerDesc.rotationY = Math::GM_PI * 0.5f;
		playerDesc.cameraDistance = 4.f;
		playerDesc.cameraYaw = Math::GM_PI * 0.5f;
		GM_ASSERT_RETURN(InitializePlayer(playerDesc), "Tutorial Player 생성에 실패했습니다.");

		GetCameraManager()->SetPixelSnapEnabled(false);
	}

	void TutorialScene::OnTick(float deltaTime)
	{
		PlayScheduledDialog();
		PlayScheduledRoadUpBGM();
		TickSceneTransitionDebug();
	}

	void TutorialScene::InitializeTutorialFlow()
	{
		GameObject* player = GetPlayer().Get();
		GM_ASSERT_RETURN(player, "Attack Tutorial을 구성하려면 Player가 필요합니다.");
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(stateMachine, "Attack Tutorial을 구성하려면 ChiStateMachineComponent가 필요합니다.");
		_playerControlComponent = player->GetComponent<PlayerControlComponent>();
		GM_ASSERT_RETURN(_playerControlComponent, "Attack Tutorial을 구성하려면 PlayerControlComponent가 필요합니다.");

		_attackGuideWidget = APPLICATION.GetUIManager().AddUserWidget<AttackGuideWidget>(*stateMachine);
		GM_ASSERT_RETURN(_attackGuideWidget, "AttackGuideWidget 생성에 실패했습니다.");
		_rhythmTutorialWidget = APPLICATION.GetUIManager().AddUserWidget<RhythmTutorialWidget>(HiFiRushStatics::GetBeatSystem(), HiFiRushStatics::GetRhythmJudge(), *stateMachine);
		GM_ASSERT_RETURN(_rhythmTutorialWidget, "RhythmTutorialWidget 생성에 실패했습니다.");
		GetDialogComponent().OnFinished.Subscribe(_dialogFinishedConnection,
			[this](const DialogFinishedEvent& event)
			{
				HandleDialogFinished(event);
			});
		GetDialogComponent().OnBranchRequested.Subscribe(_dialogBranchRequestedConnection,
			[this](const DialogBranchRequestedEvent& event)
			{
				HandleDialogBranchRequested(event);
			});
		_attackGuideWidget->OnCompleted.Subscribe(_attackGuideCompletedConnection,
			[this](const AttackGuideCompletedEvent& event)
			{
				HandleAttackGuideCompleted(event);
			});
		_rhythmTutorialWidget->OnInputPhaseChanged.Subscribe(_rhythmTutorialInputPhaseConnection,
			[this](const RhythmTutorialInputPhaseEvent& event)
			{
				HandleRhythmTutorialInputPhase(event);
			});
		_rhythmTutorialWidget->OnCompleted.Subscribe(_rhythmTutorialCompletedConnection,
			[this](const RhythmTutorialCompletedEvent& event)
			{
				HandleRhythmTutorialCompleted(event);
			});
		_tutorialPhase = TutorialPhase::IntroDialog;
	}

	void TutorialScene::HandleDialogFinished(const DialogFinishedEvent& event)
	{
		if (event.completed == false)
			return;

		if (event.sequenceId == HiFiRushDialogSequenceIds::TutorialIntro)
		{
			_tutorialPhase = TutorialPhase::WeakAttackGuide;
			_playerControlComponent->BlockControls(this, PlayerControl::Movement | PlayerControl::Camera);
			_attackGuideWidget->Show(RhythmTutorialType::Weak);
		}
		else if (event.sequenceId == HiFiRushDialogSequenceIds::TutorialWeakRhythm)
		{
			_tutorialPhase = TutorialPhase::StrongAttackIntroDialog;
			GM_ASSERT_RETURN(PlayDialogSequence(HiFiRushDialogSequenceIds::StrongAttackIntro), "강공격 안내 Dialog 재생에 실패했습니다.");
		}
		else if (event.sequenceId == HiFiRushDialogSequenceIds::StrongAttackIntro)
		{
			_tutorialPhase = TutorialPhase::StrongAttackGuide;
			_playerControlComponent->BlockControls(this, PlayerControl::Movement | PlayerControl::Camera);
			_attackGuideWidget->Show(RhythmTutorialType::Strong);
		}
		else if (event.sequenceId == HiFiRushDialogSequenceIds::StrongAttackRhythm)
		{
			_tutorialPhase = TutorialPhase::Completed;
			const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
			_roadUpBGMStartBeat = static_cast<float>(beatSystem.GetCurrentBeatIndex() + 1);
			_isRoadUpBGMQueued = true;
			GM_ASSERT_RETURN(GetTriggerSequenceSystem().Activate(RoadUpTriggerId), "Tutorial 바닥 상승 Trigger 실행에 실패했습니다.");
			_playerControlComponent->ReleaseControls(this);
		}
	}

	void TutorialScene::HandleDialogBranchRequested(const DialogBranchRequestedEvent& event)
	{
		if (event.sequence == nullptr)
			return;

		if (event.sequence->id == HiFiRushDialogSequenceIds::TutorialWeakRhythm && _tutorialPhase == TutorialPhase::WeakRhythmDialog)
		{
			_playerControlComponent->BlockControls(this, PlayerControl::All);
			_rhythmTutorialWidget->Play(RhythmTutorialType::Weak);
		}
		else if (event.sequence->id == HiFiRushDialogSequenceIds::StrongAttackRhythm && _tutorialPhase == TutorialPhase::StrongRhythmDialog)
		{
			_playerControlComponent->BlockControls(this, PlayerControl::All);
			_rhythmTutorialWidget->Play(RhythmTutorialType::Strong);
		}
	}

	void TutorialScene::HandleAttackGuideCompleted(const AttackGuideCompletedEvent& event)
	{
		if (_tutorialPhase == TutorialPhase::WeakAttackGuide && event.type == RhythmTutorialType::Weak)
		{
			_tutorialPhase = TutorialPhase::WeakRhythmDialog;
			ScheduleDialog(HiFiRushDialogSequenceIds::TutorialWeakRhythm);
		}
		else if (_tutorialPhase == TutorialPhase::StrongAttackGuide && event.type == RhythmTutorialType::Strong)
		{
			_tutorialPhase = TutorialPhase::StrongRhythmDialog;
			ScheduleDialog(HiFiRushDialogSequenceIds::StrongAttackRhythm);
		}
	}

	void TutorialScene::HandleRhythmTutorialInputPhase(const RhythmTutorialInputPhaseEvent& event)
	{
		const bool isWeakTutorial = _tutorialPhase == TutorialPhase::WeakRhythmDialog && event.type == RhythmTutorialType::Weak;
		const bool isStrongTutorial = _tutorialPhase == TutorialPhase::StrongRhythmDialog && event.type == RhythmTutorialType::Strong;
		if (isWeakTutorial == false && isStrongTutorial == false)
			return;

		const PlayerControlMask blockedControls = event.isActive ? PlayerControl::Movement | PlayerControl::Camera : PlayerControl::All;
		_playerControlComponent->BlockControls(this, blockedControls);
	}

	void TutorialScene::HandleRhythmTutorialCompleted(const RhythmTutorialCompletedEvent& event)
	{
		const bool isWeakTutorial = _tutorialPhase == TutorialPhase::WeakRhythmDialog && event.type == RhythmTutorialType::Weak;
		const bool isStrongTutorial = _tutorialPhase == TutorialPhase::StrongRhythmDialog && event.type == RhythmTutorialType::Strong;
		if (isWeakTutorial == false && isStrongTutorial == false)
			return;

		_playerControlComponent->BlockControls(this, PlayerControl::All);
		const wchar_t* branchKey = event.grade == RhythmJudgeGrade::OffBeat ? HiFiRushDialogBranchKeys::Retry : HiFiRushDialogBranchKeys::Perfect;
		GM_ASSERT_RETURN(SelectDialogBranch(branchKey), "Attack Rhythm Tutorial 결과 Dialog 선택에 실패했습니다. branch=%ls", branchKey);
	}

	void TutorialScene::ScheduleDialog(const wchar_t* sequenceId)
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		_pendingDialogSequenceId = sequenceId;
		_pendingDialogStartBeat = beatSystem.GetCurrentBeat() + 0.5f;
	}

	void TutorialScene::PlayScheduledDialog()
	{
		if (_pendingDialogSequenceId == nullptr)
			return;

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false || beatSystem.GetCurrentBeat() < _pendingDialogStartBeat)
			return;

		const wchar_t* sequenceId = _pendingDialogSequenceId;
		_pendingDialogSequenceId = nullptr;
		GM_ASSERT_RETURN(PlayDialogSequence(sequenceId), "예약된 Tutorial Dialog 재생에 실패했습니다. sequence=%ls", sequenceId);
	}

	void TutorialScene::PlayScheduledRoadUpBGM()
	{
		if (_isRoadUpBGMQueued == false)
			return;

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false || beatSystem.GetCurrentBeat() < _roadUpBGMStartBeat)
			return;

		_isRoadUpBGMQueued = false;
		TransitionRhythmBGM(HiFiRushBGM::TutorialRoadUp);
		SetGameplayStatusUIVisible(true);
	}

	void TutorialScene::InitializeSubObject()
	{
		auto background = SpawnGameObject<GameObject>({ 0.f, 0.f, 500.f });
		auto spriteComponent = background->AddComponent<SpriteComponent>();
		auto texture = APPLICATION.GetResources().Find<Texture>(L"Xanadu");
		spriteComponent->SetTexture(texture);
		auto transform = background->GetTransform();
		transform->SetScale(Vector2{ static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()) });

		for (int i = 0; i < 20; ++i)
		{
			auto monster = SpawnGameObject<GameObject>({ 200.f * i, 300.f, 0.f });
			auto monsterSprite = monster->AddComponent<SpriteComponent>();
			auto monsterTexture = APPLICATION.GetResources().Find<Texture>(L"OrangeMushroom");
			monsterSprite->SetTexture(monsterTexture);

			auto monsterTransform = monster->GetTransform();
			monsterTransform->SetScale(Vector2{ static_cast<float>(monsterTexture->GetWidth()), static_cast<float>(monsterTexture->GetHeight()) });
		}

		auto ground = SpawnGameObject<GameObject>({ 0.f, -100.f, 0.f });
		BoxCollider2DComponent* groundCollider = ground->AddComponent<BoxCollider2DComponent>();
		groundCollider->SetSize({ 120000.f, 100.f });
	}

	void TutorialScene::InitializeStaticMeshTest()
	{
		std::shared_ptr<StaticMesh> staticMesh = APPLICATION.GetResources().Find<StaticMesh>(L"Environment97");
		GM_ASSERT_RETURN(staticMesh, "Environment97 StaticMesh가 로드되지 않았습니다.");

		GameObject* testObject = SpawnGameObject<GameObject>({ 350.f, 0.f, 100.f });
		TransformComponent* transform = testObject->GetTransform();
		transform->SetPosition(Vector3{ 350.f, -200.f, 10.f });
		transform->SetScale(Vector3{ 1000.f, 1000.f, 1000.f });

		StaticMeshComponent* staticMeshComponent = testObject->AddComponent<StaticMeshComponent>();
		staticMeshComponent->SetStaticMesh(staticMesh);
	}

	void TutorialScene::InitializeSkeletalMeshTest()
	{
		std::shared_ptr<SkeletalMesh> skeletalMesh = APPLICATION.GetResources().Find<SkeletalMesh>(L"chi");
		GM_ASSERT_RETURN(skeletalMesh, "chi SkeletalMesh가 로드되지 않았습니다.");

		GameObject* testObject = SpawnGameObject<GameObject>({ -350.f, -200.f, 200.f });
		TransformComponent* transform = testObject->GetTransform();
		transform->SetScale(Vector3{ 250.f, 250.f, 250.f });
		transform->SetRotationY(Math::GM_PI);

		SkeletalMeshComponent* skeletalMeshComponent = testObject->AddComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
		SkeletalAnimatorComponent* animatorComponent = testObject->AddComponent<SkeletalAnimatorComponent>();

		std::shared_ptr<SkeletalAnimationClip> animationClip = APPLICATION.GetResources().Find<SkeletalAnimationClip>(L"chi.DefaultAnimation");
		GM_ASSERT_RETURN(animationClip, "chi.DefaultAnimation SkeletalAnimationClip이 로드되지 않았습니다.");
		animatorComponent->AddClip(L"Default", animationClip);
		animatorComponent->Play(L"Default");
	}
}
