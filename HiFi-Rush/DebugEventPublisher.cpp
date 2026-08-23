#include "DebugEventPublisher.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "DebugInputHandler.h"
#include "EnvironmentModelDebugLabelComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"
#include "IDebugRenderer.h"
#include "GMLog.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"
#include "UIManager.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t DebugInputType[] = L"HiFiRush.DebugEvent";
		constexpr float DebugAreaAttackRadius = 40.f;
		constexpr int32 DebugAreaAttackDamage = 100;

		void SpawnDebugAreaAttack()
		{
			GameplayScene* scene = dynamic_cast<GameplayScene*>(APPLICATION.GetSceneManager().GetActiveScene());
			if (scene == nullptr)
				return;

			GameObject* player = scene->GetPlayer().Get();
			if (player == nullptr || player->GetTransform() == nullptr)
				return;

			TemporarySphereHitBoxDesc desc{};
			desc.world = Matrix::CreateTranslation(player->GetTransform()->GetPosition());
			desc.colliderId = L"Debug.PlayerAreaAttack";
			desc.radius = DebugAreaAttackRadius;
			desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
			desc.collisionMask = HiFiRushCollisionLayer::Monster;
			desc.damageInfo.amount = DebugAreaAttackDamage;
			desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
			desc.lifetime = 0.1f;
			if (scene->SpawnGameObject<TemporaryHitBoxObject>(desc))
				GM_LOG("[Gameplay Debug] Player Area Attack: Radius=%.1f, Damage=%d", DebugAreaAttackRadius, DebugAreaAttackDamage);
		}
	}

	void DebugEventPublisher::Initialize()
	{
		DebugInputHandler::RegisterDebugType(DebugInputType, true);
	}

	void DebugEventPublisher::Tick()
	{
		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F11))
		{
			IDebugRenderer& debugRenderer = APPLICATION.GetDebugRenderer();
			const bool isEnabled = debugRenderer.IsEnabled() == false;
			debugRenderer.SetEnabled(isEnabled);
			APPLICATION.GetUIManager().SetDebugWidgetsVisible(isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::B))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsFrustumCullingEnabled() == false;
				renderer.SetFrustumCullingEnabled(isEnabled);
			}
			else
			{
				const bool isEnabled = renderer.IsBoundingVolumeDebugDrawEnabled() == false;
				renderer.SetBoundingVolumeDebugDrawEnabled(isEnabled);
			}
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::I))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsStaticMeshInstancingEnabled() == false;
				renderer.SetStaticMeshInstancingEnabled(isEnabled);
			}
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::C))
		{
			PhysicsSystem& physicsSystem = APPLICATION.GetPhysicsSystem();
			const bool isEnabled = physicsSystem.IsCollider3DDebugDrawEnabled();
			physicsSystem.SetCollider3DDebugDrawEnabled(!isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::N))
		{
			NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
			const bool isEnabled = navMeshSystem.IsDebugDrawEnabled();
			navMeshSystem.SetDebugDrawEnabled(!isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::M))
		{
			EnvironmentModelDebugLabelComponent::ToggleLabels();
			GM_LOG("[Environment Debug] Model Labels: %s", EnvironmentModelDebugLabelComponent::AreLabelsVisible() ? "On" : "Off");
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::H))
			SpawnDebugAreaAttack();

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F9))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);
			if (isControlPressed)
				renderer.SetRenderTargetDebugView(RenderTargetDebugView::OriginalScene);
			else
				renderer.SelectNextRenderTargetDebugView();
		}

	}
}

#endif
