#include "QamilScene.h"
#include "Application.h"
#include "CameraManager.h"
#include "HiFiRushAudio.h"
#include "Input.h"
#include "MathUtil.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSpawner.h"
#include "Resources.h"
#include "SceneDebugTools.h"

namespace gm
{
	void QamilScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);

		const std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"qamil");
		GM_ASSERT_RETURN(navigationMesh, "qamil NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);

		APPLICATION.GetInput().SetCursorLocked(true);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Qamil);
		InitializeGameplayUI();
	}

	void QamilScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void QamilScene::OnInitialize()
	{
		GM_ASSERT_RETURN(InitializeMap(L"QamilMap"), "Qamil Map 구성에 실패했습니다.");

		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ -0.43f, 0.f, -6.13f };
		playerDesc.cameraDistance = 5.f;
		playerDesc.cameraYaw = Math::DegreesToRadians(-90.f);
		playerDesc.cameraPitch = Math::DegreesToRadians(10.f);
		playerDesc.cameraHeight = 3.f;
		GM_ASSERT_RETURN(InitializePlayer(playerDesc), "Qamil Player 생성에 실패했습니다.");
	}

	void QamilScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
	}

}
