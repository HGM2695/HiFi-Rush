#include "DebugTextWidget.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "BeatSystem.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "TextBlock.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace gm
{
	namespace
	{
		constexpr uint32 BeatProgressBarWidth = 20;
		constexpr float BeatPulseDuration = 0.1f;

		std::wstring CreateBeatProgressBar(float progress)
		{
			const uint32 filledCount = static_cast<uint32>(progress * BeatProgressBarWidth);
			std::wstring progressBar(BeatProgressBarWidth, L'-');
			std::fill_n(progressBar.begin(), std::min(filledCount, BeatProgressBarWidth), L'#');
			return progressBar;
		}
	}

	std::unique_ptr<Widget> DebugTextWidget::BuildWidgetTree()
	{
		auto textBlock = CreateRootWidget<TextBlock>();
		textBlock->SetPosition(Vector2{ 0.f, 0.f });
		textBlock->SetColor(Colors::Green);

		return textBlock;
	}

	void DebugTextWidget::OnTick(float deltaTime)
	{
		_accTime += deltaTime;
		++_callCount;

		if (_accTime >= 1.f)
		{
			_fps = static_cast<float>(_callCount) / _accTime;
			_callCount = 0;
			_accTime = 0.f;
		}

		if (_beatSystem.DidCrossBeatBoundary())
			_beatPulseTime = BeatPulseDuration;
		else
			_beatPulseTime = std::max(0.f, _beatPulseTime - deltaTime);

		TextBlock* textBlock = static_cast<TextBlock*>(GetRootWidget());
		textBlock->SetColor(_beatPulseTime > 0.f ? Colors::Red : Colors::Green);

		std::wostringstream text;
		text << std::fixed << std::setprecision(1);
		text << L"FPS : " << _fps << L'\n';
		text << L"BPM : " << _beatSystem.GetBPM() << L'\n';

		if (_beatSystem.HasPlaybackTime() == false)
		{
			text << L"BGM Time : Not Playing\n";
			text << L"Beat : -\n";
			text << L"Beat Progress : [--------------------] 0.0%";
		}
		else
		{
			text << std::setprecision(3);
			text << L"BGM Time : " << _beatSystem.GetPlaybackTime() << L" s\n";
			text << L"Beat : " << _beatSystem.GetCurrentBeatIndex() << L" (" << _beatSystem.GetCurrentBeat() << L")\n";
			text << std::setprecision(1);
			text << L"Beat Progress : [" << CreateBeatProgressBar(_beatSystem.GetBeatProgress()) << L"] " << _beatSystem.GetBeatProgress() * 100.f << L'%';
		}

		const Renderer& renderer = APPLICATION.GetRenderer();
		const PhysicsSystem& physicsSystem = APPLICATION.GetPhysicsSystem();
		const CullingDebugStats cullingStats = renderer.GetCullingDebugStats();
		const StaticMeshInstancingDebugStats instancingStats = renderer.GetStaticMeshInstancingDebugStats();
		const uint32 submittedCount = cullingStats.staticMesh.submittedCount + cullingStats.skeletalMesh.submittedCount;
		const uint32 visibleCount = cullingStats.staticMesh.visibleCount + cullingStats.skeletalMesh.visibleCount;
		const uint32 culledCount = cullingStats.staticMesh.culledCount + cullingStats.skeletalMesh.culledCount;

		text << L"\nBounding Volume [B] : " << (renderer.IsBoundingVolumeDebugDrawEnabled() ? L"On" : L"Off");
		text << L"\n3D Collider [C] : " << (physicsSystem.IsCollider3DDebugDrawEnabled() ? L"On" : L"Off");
		text << L"\nFrustum Culling [Ctrl+B] : " << (renderer.IsFrustumCullingEnabled() ? L"On" : L"Off");
		text << L"\nStatic Mesh Instancing [Ctrl+I] : " << (renderer.IsStaticMeshInstancingEnabled() ? L"On" : L"Off");
		text << L"\nStatic Mesh Batches : " << instancingStats.renderBatchCount;
		text << L"\nStatic Mesh Draw Calls (Normal / Instanced) : " << instancingStats.normalDrawCallCount << L" / " << instancingStats.instancedDrawCallCount;
		text << L"\nInstanced Objects : " << instancingStats.instancedInstanceCount;
		text << L"\nCulling Stats (Submitted / Visible / Culled)";
		text << L"\n  Total : " << submittedCount << L" / " << visibleCount << L" / " << culledCount;
		text << L"\n  Static : " << cullingStats.staticMesh.submittedCount << L" / " << cullingStats.staticMesh.visibleCount << L" / " << cullingStats.staticMesh.culledCount;
		text << L"\n  Skeletal : " << cullingStats.skeletalMesh.submittedCount << L" / " << cullingStats.skeletalMesh.visibleCount << L" / " << cullingStats.skeletalMesh.culledCount;

		textBlock->SetText(text.str());
	}
}

#endif
