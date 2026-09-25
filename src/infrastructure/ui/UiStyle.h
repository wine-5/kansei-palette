#pragma once
#include "raylib.h"
#include "game/flow/GameFlow.h"

// UI の色と、文字を描く共通の関数
namespace infrastructure::ui
{
	// --- 色 ---
	constexpr Color ACCENT_INITIAL{ 0x3a, 0x3a, 0x3a, 255 }; // 色が 1 つも戻っていないときの強調色
	constexpr Color ACCENT_RED{ 0xe5, 0x49, 0x3f, 255 };
	constexpr Color ACCENT_BLUE{ 0x3d, 0x8b, 0xea, 255 };
	constexpr Color ACCENT_GREEN{ 0x4c, 0xb8, 0x5a, 255 };
	constexpr Color HUE_DOT_COLORS[]{
		{ 0xe5, 0x49, 0x3f, 255 }, // 赤
		{ 0x3d, 0x8b, 0xea, 255 }, // 青
		{ 0x8b, 0xc3, 0x4a, 255 }, // 黄緑
	};
	constexpr Color DOT_EMPTY_COLOR{ 0xb8, 0xb8, 0xb8, 255 }; // まだ戻っていない色の丸
	constexpr Color PANEL_COLOR{ 20, 20, 24, 150 };             // 文字の背景の半透明の板
	constexpr Color TEXT_COLOR{ 250, 250, 250, 255 };
	constexpr Color TEXT_SUB_COLOR{ 220, 220, 220, 255 };
	constexpr Color TEXT_OUTLINE_COLOR{ 30, 30, 34, 200 };
	constexpr float TEXT_SPACING{ 1.0f };

	/// 世界に戻った色の数(0〜3)
	inline int countRestoredColors(const game::flow::GameFlow& flow)
	{
		const game::flow::RestoreLevel& restore{ flow.getRestore() };
		return (restore.m_red > 0.5f ? 1 : 0) + (restore.m_blue > 0.5f ? 1 : 0) + (restore.m_yellowGreen > 0.5f ? 1 : 0);
	}

	/// 強調色(ステージをクリアするごとに 赤 → 青 → 緑、エンディングは赤)
	inline Color accentColor(const game::flow::GameFlow& flow)
	{
		if (flow.getPhase() == game::flow::GamePhase::Ending)
			return ACCENT_RED;
		switch (countRestoredColors(flow))
		{
		case 0: return ACCENT_INITIAL;
		case 1: return ACCENT_RED;
		case 2: return ACCENT_BLUE;
		default: return ACCENT_GREEN;
		}
	}

	/// 文字の幅と高さ
	inline Vector2 measureText(const Font& font, const char* text, float size)
	{
		return MeasureTextEx(font, text, size, TEXT_SPACING);
	}

	/// 文字を描く(左上の位置を指定)
	inline void drawText(const Font& font, const char* text, Vector2 position, float size, Color color)
	{
		DrawTextEx(font, text, position, size, TEXT_SPACING, color);
	}

	/// 文字を横中央そろえで描く
	inline void drawTextCentered(const Font& font, const char* text, float centerX, float y, float size, Color color)
	{
		const Vector2 textSize{ measureText(font, text, size) };
		drawText(font, text, Vector2{ centerX - textSize.x / 2.0f, y }, size, color);
	}

	/// 縁取りを付けて文字を描く(背景の絵が細かくても読めるように)
	inline void drawTextOutlined(const Font& font, const char* text, Vector2 position, float size, Color color, float outline = 2.0f)
	{
		for (int dy{ -1 }; dy <= 1; ++dy)
		{
			for (int dx{ -1 }; dx <= 1; ++dx)
			{
				if (dx != 0 || dy != 0)
					drawText(font, text, Vector2{ position.x + dx * outline, position.y + dy * outline }, size, TEXT_OUTLINE_COLOR);
			}
		}
		drawText(font, text, position, size, color);
	}

	/// 角の丸い半透明の板(文字の背景)
	inline void drawPanel(Rectangle bounds)
	{
		DrawRectangleRounded(bounds, 0.3f, 8, PANEL_COLOR);
	}
} // namespace infrastructure::ui
