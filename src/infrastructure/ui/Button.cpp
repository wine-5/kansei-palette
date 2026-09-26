#include "Button.h"
#include "UiScale.h"
#include "UiStyle.h"

namespace
{
	constexpr float PRESS_SINK{ 3.0f };      // 押しているときに沈む量(ピクセル)
	constexpr float SHADOW_OFFSET{ 4.0f };   // 下に敷く影の厚み
	constexpr float LABEL_SIZE_RATIO{ 0.5f }; // ボタンの高さに対する文字の大きさ
	constexpr float ROUNDNESS{ 1.0f };       // 1.0 = 両端が半円(カプセル型)
	constexpr int SEGMENTS{ 16 };

	/// 色を明るく(amount > 0)または暗く(amount < 0)する
	Color shade(Color color, float amount)
	{
		const auto adjust{ [amount](unsigned char c) {
			const float value{ amount >= 0.0f ? c + (255 - c) * amount : c * (1.0f + amount) };
			return static_cast<unsigned char>(value);
		} };
		return Color{ adjust(color.r), adjust(color.g), adjust(color.b), color.a };
	}
} // namespace

namespace infrastructure::ui
{
	bool Button::isClicked(Rectangle bounds)
	{
		// 押した瞬間に反応させる(タッチでも待たされない)
		return isHovered(bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	}

	bool Button::isHovered(Rectangle bounds)
	{
		return CheckCollisionPointRec(uiMousePosition(), bounds);
	}

	void Button::draw(Rectangle bounds, const char* label, const Font& font, Color accent)
	{
		const bool isHover{ isHovered(bounds) };
		const bool isPressed{ isHover && IsMouseButtonDown(MOUSE_BUTTON_LEFT) };

		// 影(下に敷いた濃い色)の上に本体を載せ、押しているときは本体を沈める
		const Rectangle shadow{ bounds.x, bounds.y + SHADOW_OFFSET, bounds.width, bounds.height };
		DrawRectangleRounded(shadow, ROUNDNESS, SEGMENTS, shade(accent, -0.45f));
		Rectangle body{ bounds };
		if (isPressed)
			body.y += PRESS_SINK;
		DrawRectangleRounded(body, ROUNDNESS, SEGMENTS, isHover ? shade(accent, 0.15f) : accent);

		const float size{ bounds.height * LABEL_SIZE_RATIO };
		const Vector2 textSize{ measureText(font, label, size) };
		drawText(font, label, Vector2{ body.x + (body.width - textSize.x) / 2.0f, body.y + (body.height - textSize.y) / 2.0f }, size, TEXT_COLOR);
	}
} // namespace infrastructure::ui
