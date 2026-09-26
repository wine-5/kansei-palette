#include "ClearCard.h"
#include "Button.h"
#include "UiScale.h"
#include "UiStyle.h"
#include "UiText.h"
#include "game/data/Stages.h"
#include "infrastructure/resource/Assets.h"
#include <cstdio>
#include <string>

namespace
{
	using infrastructure::ui::logicalHeight;
	using infrastructure::ui::logicalWidth;

	constexpr float CARD_WIDTH{ 560.0f };
	constexpr float CARD_HEIGHT{ 200.0f };
	constexpr float CARD_BOTTOM_MARGIN{ 24.0f };
	constexpr float DOT_RADIUS{ 26.0f };
	constexpr float TITLE_SIZE{ 36.0f };
	constexpr float SUB_SIZE{ 22.0f };
	constexpr float BUTTON_WIDTH{ 200.0f };
	constexpr float BUTTON_HEIGHT{ 56.0f };

	Rectangle cardBounds()
	{
		return Rectangle{ (logicalWidth() - CARD_WIDTH) / 2.0f, logicalHeight() - CARD_BOTTOM_MARGIN - CARD_HEIGHT, CARD_WIDTH, CARD_HEIGHT };
	}

	Rectangle nextButtonBounds()
	{
		const Rectangle card{ cardBounds() };
		return Rectangle{ card.x + (card.width - BUTTON_WIDTH) / 2.0f, card.y + card.height - BUTTON_HEIGHT - 22.0f, BUTTON_WIDTH, BUTTON_HEIGHT };
	}
} // namespace

namespace infrastructure::ui
{
	UiAction ClearCard::update(const game::flow::GameFlow& flow)
	{
		(void)flow;
		UiAction action{};
		action.m_isPointerOverUi = Button::isHovered(nextButtonBounds());
		action.m_isConfirmClicked = Button::isClicked(nextButtonBounds());
		return action;
	}

	void ClearCard::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		const Font& font{ assets.getUiFont() };
		const int stageIndex{ flow.getStageIndex() };
		const Color stageColor{ HUE_DOT_COLORS[static_cast<int>(game::data::STAGES[stageIndex].m_hue)] };
		const Rectangle card{ cardBounds() };
		drawPanel(card);

		// 「○○が もどった」の左に、戻った色の丸を置く
		const std::string title{ std::string(game::data::STAGES[stageIndex].m_name) + TEXT_COLOR_RETURNED };
		const Vector2 titleSize{ measureText(font, title.c_str(), TITLE_SIZE) };
		const float rowWidth{ DOT_RADIUS * 2.0f + 18.0f + titleSize.x };
		const float left{ card.x + (card.width - rowWidth) / 2.0f };
		const float titleY{ card.y + 24.0f };
		DrawCircleV(Vector2{ left + DOT_RADIUS, titleY + TITLE_SIZE / 2.0f }, DOT_RADIUS, stageColor);
		drawText(font, title.c_str(), Vector2{ left + DOT_RADIUS * 2.0f + 18.0f, titleY }, TITLE_SIZE, TEXT_COLOR);

		// 「あと N つの いろ」
		const int remaining{ static_cast<int>(game::data::STAGES.size()) - (stageIndex + 1) };
		char remainingText[64]{};
		std::snprintf(remainingText, sizeof(remainingText), TEXT_COLORS_LEFT, remaining);
		drawTextCentered(font, remainingText, card.x + card.width / 2.0f, titleY + TITLE_SIZE + 10.0f, SUB_SIZE, TEXT_SUB_COLOR);

		Button::draw(nextButtonBounds(), TEXT_NEXT, font, stageColor);
	}
} // namespace infrastructure::ui
