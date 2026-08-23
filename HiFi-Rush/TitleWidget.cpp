#include "TitleWidget.h"
#include "Application.h"
#include "CanvasPanel.h"
#include "Image.h"
#include "IGraphicsResourceFactory.h"
#include "Paths.h"
#include "Texture.h"
#include "TitleResources.h"

#include <algorithm>

namespace gm
{
	std::unique_ptr<Widget> TitleWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(L"TitleRoot");
		root->SetSizeRule(WidgetSizeRule::FillParent);

		const Vector2 screenSize{ static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()) };
		_background = root->AddNamedChild<Image>(L"TitleBackground");
		_background->SetGeometry(screenSize * 0.5f, screenSize);
		return root;
	}

	void TitleWidget::OnInitialize()
	{
		GM_ASSERT_RETURN(_background, "TitleWidget의 배경 Image가 없습니다.");

		IGraphicsResourceFactory& resourceFactory = APPLICATION.GetGraphicsResourceFactory();
		_frames.reserve(TitleResource::FrameCount);
		for (uint32 frameIndex = 0; frameIndex < TitleResource::FrameCount; ++frameIndex)
		{
			TextureLoadDesc desc{};
			desc.path = GetTexturePath(TitleResource::GetFrameTexturePath(frameIndex));
			desc.colorSpace = TextureColorSpace::SRGB;

			std::shared_ptr<Texture> frame = resourceFactory.LoadTexture(desc);
			GM_ASSERT_RETURN(frame, "Title Texture 생성에 실패했습니다. frame=%u", frameIndex);
			_frames.push_back(std::move(frame));
		}

		_background->SetTexture(_frames.front());
		_elapsedTime = 0.f;
		_currentFrameIndex = 0;
	}

	void TitleWidget::OnTick(float deltaTime)
	{
		if (_frames.empty())
			return;

		_elapsedTime += deltaTime;
		const uint32 frameIndex = std::min(static_cast<uint32>(_elapsedTime * TitleResource::FrameRate), TitleResource::FrameCount - 1);
		if (frameIndex == _currentFrameIndex)
			return;

		_currentFrameIndex = frameIndex;
		_background->SetTexture(_frames[_currentFrameIndex]);
	}
}
