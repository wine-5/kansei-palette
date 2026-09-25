#include "Button.h"

namespace infrastructure::ui
{
	bool Button::isClicked(Rectangle bounds)
	{
		// TODO: 実装する(離した瞬間に判定するか、押した瞬間にするかを決める)
		return isHovered(bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	}

	bool Button::isHovered(Rectangle bounds)
	{
		return CheckCollisionPointRec(GetMousePosition(), bounds);
	}

	void Button::draw(Rectangle bounds, const char* label, const Font& font, Color accent)
	{
		// TODO: 実装する(カプセル型、押下中は 3px 沈める、文字を中央に)
		(void)label;
		(void)font;
		DrawRectangleRoundedLines(bounds, 1.0f, 8, accent);
	}
} // namespace infrastructure::ui
