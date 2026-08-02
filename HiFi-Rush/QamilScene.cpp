#include "QamilScene.h"
#include "Application.h"
#include "BinaryEnvironmentMapLoader.h"
#include "EnvironmentMapTypes.h"
#include "EnvironmentSpawner.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "Paths.h"
#include "PhysicsSystem.h"
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

		ActivateEnvironmentOverviewCamera(*this);
	}

	void QamilScene::OnInitialize()
	{
		InitializeEnvironment();
	}

	void QamilScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
	}

	void QamilScene::InitializeEnvironment()
	{
		EnvironmentMapData mapData{};
		GM_ASSERT_RETURN(BinaryEnvironmentMapLoader::Load(GetMapPath(L"QamilEnvironmentMap.bin"), mapData), "Qamil 환경 맵을 로드하지 못했습니다.");

		EnvironmentSpawner spawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN(spawner.Spawn(*this, mapData), "Qamil 환경 오브젝트 생성에 실패했습니다.");
		CreateEnvironmentOverviewCamera(*this, Vector3{ -0.43f, 4.f, -15.f }, Vector3{ 0.f, 1.f, 0.f });
	}
}
