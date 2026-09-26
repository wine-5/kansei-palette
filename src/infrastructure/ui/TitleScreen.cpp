#include "TitleScreen.h"
#include "Button.h"
#include "UiScale.h"
#include "UiStyle.h"
#include "UiText.h"
#include "infrastructure/resource/Assets.h"
#include <algorithm>
#include <iterator>
#include <string>

namespace
{
	using infrastructure::ui::logicalHeight;
	using infrastructure::ui::logicalWidth;

	// タイトルの文字の大きさ(画面幅の 7.2%、34〜84px)
	constexpr float TITLE_SIZE_RATIO{ 0.072f };
	constexpr float TITLE_SIZE_MIN{ 34.0f };
	constexpr float TITLE_SIZE_MAX{ 84.0f };
	constexpr float TITLE_Y{ 36.0f };
	constexpr float SUBTITLE_SIZE{ 26.0f };
	constexpr float CAPTION_SIZE{ 22.0f };
	constexpr Color TITLE_GRAY{ 0x8a, 0x8a, 0x8a, 255 };

	// エンディングでタイトルの文字が左から順に色づく
	constexpr float TITLE_COLOR_DELAY{ 0.11f }; // 1 文字ごとの遅れ(秒)
	constexpr float TITLE_COLOR_TIME{ 1.1f };   // 1 文字が色づくまでの時間(秒)
	constexpr Color TITLE_COLORS[]{
		{ 0xe5, 0x49, 0x3f, 255 }, { 0xf0, 0x8a, 0x2c, 255 }, { 0xf3, 0xc2, 0x1f, 255 },
		{ 0x8b, 0xc3, 0x4a, 255 }, { 0x4c, 0xb8, 0x5a, 255 }, { 0x2f, 0xb5, 0xc9, 255 },
		{ 0x3d, 0x8b, 0xea, 255 }, { 0x7b, 0x5c, 0xd6, 255 }, { 0xc2, 0x57, 0xc4, 255 },
	};

	constexpr float BUTTON_WIDTH{ 280.0f };
	constexpr float BUTTON_HEIGHT{ 64.0f };
	constexpr float BUTTON_BOTTOM_MARGIN{ 56.0f };

	Rectangle buttonBounds()
	{
		return Rectangle{ (logicalWidth() - BUTTON_WIDTH) / 2.0f, logicalHeight() - BUTTON_BOTTOM_MARGIN - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT };
	}

	Color lerpColor(Color from, Color to, float t)
	{
		const auto mix{ [t](unsigned char a, unsigned char b) { return static_cast<unsigned char>(a + (b - a) * t); } };
		return Color{ mix(from.r, to.r), mix(from.g, to.g), mix(from.b, to.b), 255 };
	}
} // namespace

namespace infrastructure::ui
{
	UiAction TitleScreen::update(const game::flow::GameFlow& flow)
	{
		(void)flow;
		UiAction action{};
		action.m_isPointerOverUi = Button::isHovered(buttonBounds());
		action.m_isConfirmClicked = Button::isClicked(buttonBounds());
		return action;
	}

	void TitleScreen::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		const bool isEnding{ flow.getPhase() == game::flow::GamePhase::Ending };
		const float centerX{ logicalWidth() / 2.0f };

		// タイトル: 1 文字ずつ描く(エンディングでは左から順に色づく)
		const Font& titleFont{ assets.getTitleFont() };
		const float titleSize{ std::clamp(logicalWidth() * TITLE_SIZE_RATIO, TITLE_SIZE_MIN, TITLE_SIZE_MAX) };
		const Vector2 titleWidth{ measureText(titleFont, TEXT_TITLE, titleSize) };
		float x{ centerX - titleWidth.x / 2.0f };
		int codepointCount{};
		int* codepoints{ LoadCodepoints(TEXT_TITLE, &codepointCount) };
		for (int i{}; i < codepointCount; ++i)
		{
			int byteCount{};
			const char* character{ CodepointToUTF8(codepoints[i], &byteCount) };
			const std::string glyph(character, byteCount);

			Color color{ TITLE_GRAY };
			if (isEnding)
			{
				const float t{ std::clamp((flow.getPhaseTime() - i * TITLE_COLOR_DELAY) / TITLE_COLOR_TIME, 0.0f, 1.0f) };
				color = lerpColor(TITLE_GRAY, TITLE_COLORS[i % std::size(TITLE_COLORS)], t);
			}
			drawTextOutlined(titleFont, glyph.c_str(), Vector2{ x, TITLE_Y }, titleSize, color, 3.0f);
			x += measureText(titleFont, glyph.c_str(), titleSize).x + TEXT_SPACING;
		}
		UnloadCodepoints(codepoints);

		// サブタイトルとキャプション(エンディングでは言葉が変わる)
		const Font& font{ assets.getUiFont() };
		const char* subtitle{ isEnding ? TEXT_ENDING_SUBTITLE : TEXT_SUBTITLE };
		const char* caption{ isEnding ? TEXT_ENDING_CAPTION : TEXT_CAPTION };
		const float subtitleY{ TITLE_Y + titleSize + 12.0f };
		const Vector2 subtitleSize{ measureText(font, subtitle, SUBTITLE_SIZE) };
		drawTextOutlined(font, subtitle, Vector2{ centerX - subtitleSize.x / 2.0f, subtitleY }, SUBTITLE_SIZE, TEXT_COLOR);
		const Vector2 captionSize{ measureText(font, caption, CAPTION_SIZE) };
		drawTextOutlined(font, caption, Vector2{ centerX - captionSize.x / 2.0f, subtitleY + SUBTITLE_SIZE + 10.0f }, CAPTION_SIZE, TEXT_SUB_COLOR);

		Button::draw(buttonBounds(), isEnding ? TEXT_RETRY : TEXT_START, font, accentColor(flow));
	}
} // namespace infrastructure::ui
