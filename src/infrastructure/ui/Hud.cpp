#include "Hud.h"
#include "Button.h"
#include "UiStyle.h"
#include "UiText.h"
#include "game/data/Stages.h"
#include "infrastructure/resource/Assets.h"
#include <cstdio>

namespace
{
	constexpr float MARGIN{ 16.0f };

	// 左上: ステージ番号・名前・取り戻した色の丸
	constexpr Rectangle STAGE_PANEL{ MARGIN, MARGIN, 330.0f, 92.0f };
	constexpr float STAGE_LABEL_SIZE{ 20.0f };
	constexpr float STAGE_NAME_SIZE{ 32.0f };
	constexpr float DOT_RADIUS{ 11.0f };
	constexpr float DOT_RADIUS_FILLED{ 14.0f }; // 取り戻した色は少し大きくする
	constexpr float DOT_GAP{ 36.0f };

	// 右上: ボタン
	constexpr float BUTTON_HEIGHT{ 48.0f };
	constexpr float RESET_BUTTON_WIDTH{ 210.0f };
	constexpr float SOUND_BUTTON_WIDTH{ 130.0f };
	constexpr float BUTTON_GAP{ 12.0f };

	// 下: ヒント文とゴールの点灯数
	constexpr float BOTTOM_PANEL_WIDTH{ 900.0f };
	constexpr float BOTTOM_PANEL_HEIGHT{ 58.0f };
	constexpr float HINT_SIZE{ 22.0f };
	constexpr float GOAL_SIZE{ 26.0f };

	Rectangle soundButtonBounds()
	{
		return Rectangle{ GetScreenWidth() - MARGIN - SOUND_BUTTON_WIDTH, MARGIN, SOUND_BUTTON_WIDTH, BUTTON_HEIGHT };
	}

	Rectangle resetButtonBounds()
	{
		const Rectangle sound{ soundButtonBounds() };
		return Rectangle{ sound.x - BUTTON_GAP - RESET_BUTTON_WIDTH, MARGIN, RESET_BUTTON_WIDTH, BUTTON_HEIGHT };
	}

	bool canReset(const game::flow::GameFlow& flow)
	{
		return flow.getPhase() == game::flow::GamePhase::Playing;
	}
} // namespace

namespace infrastructure::ui
{
	UiAction Hud::update(const game::flow::GameFlow& flow)
	{
		UiAction action{};
		action.m_isSoundToggleClicked = Button::isClicked(soundButtonBounds());
		action.m_isPointerOverUi = Button::isHovered(soundButtonBounds());
		if (canReset(flow))
		{
			action.m_isResetClicked = Button::isClicked(resetButtonBounds());
			action.m_isPointerOverUi = action.m_isPointerOverUi || Button::isHovered(resetButtonBounds());
		}
		return action;
	}

	void Hud::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		const Font& font{ assets.getUiFont() };
		const game::data::StageDefinition& stage{ game::data::STAGES[flow.getStageIndex()] };
		const Color accent{ accentColor(flow) };

		// 左上: STAGE n / 3、ステージ名、取り戻した色の丸 3 つ
		drawPanel(STAGE_PANEL);
		char stageLabel[32]{};
		std::snprintf(stageLabel, sizeof(stageLabel), "STAGE %d / %d", flow.getStageIndex() + 1, static_cast<int>(game::data::STAGES.size()));
		drawText(font, stageLabel, Vector2{ STAGE_PANEL.x + 18.0f, STAGE_PANEL.y + 12.0f }, STAGE_LABEL_SIZE, TEXT_SUB_COLOR);
		drawText(font, stage.m_name, Vector2{ STAGE_PANEL.x + 18.0f, STAGE_PANEL.y + 40.0f }, STAGE_NAME_SIZE, TEXT_COLOR);

		const game::flow::RestoreLevel& restore{ flow.getRestore() };
		const float restored[]{ restore.m_red, restore.m_blue, restore.m_yellowGreen };
		for (int i{}; i < 3; ++i)
		{
			const Vector2 center{ STAGE_PANEL.x + STAGE_PANEL.width - 24.0f - (2 - i) * DOT_GAP, STAGE_PANEL.y + STAGE_PANEL.height / 2.0f };
			const bool isFilled{ restored[i] > 0.5f };
			DrawCircleV(center, isFilled ? DOT_RADIUS_FILLED : DOT_RADIUS, isFilled ? HUE_DOT_COLORS[i] : DOT_EMPTY_COLOR);
		}

		// 右上: やりなおす(遊んでいるときだけ)、音の ON/OFF
		if (canReset(flow))
			Button::draw(resetButtonBounds(), TEXT_RESET, font, accent);
		Button::draw(soundButtonBounds(), flow.isSoundOn() ? TEXT_SOUND_ON : TEXT_SOUND_OFF, font, accent);

		// 下: ヒント文と「ゴール 点灯数 / 全体」(クリアカードを出している間は隠す)
		if (flow.getPhase() == game::flow::GamePhase::ClearCard)
			return;
		const Rectangle bottom{ (GetScreenWidth() - BOTTOM_PANEL_WIDTH) / 2.0f, GetScreenHeight() - MARGIN - BOTTOM_PANEL_HEIGHT, BOTTOM_PANEL_WIDTH, BOTTOM_PANEL_HEIGHT };
		drawPanel(bottom);
		const float textY{ bottom.y + (bottom.height - HINT_SIZE) / 2.0f };
		drawText(font, stage.m_hint, Vector2{ bottom.x + 24.0f, textY }, HINT_SIZE, TEXT_COLOR);

		const game::board::Board& board{ flow.getBoard() };
		char goalLabel[64]{};
		std::snprintf(goalLabel, sizeof(goalLabel), "%s %d / %d", TEXT_GOAL, board.countLitGoals(), board.countGoals());
		const Vector2 goalSize{ measureText(font, goalLabel, GOAL_SIZE) };
		drawText(font, goalLabel, Vector2{ bottom.x + bottom.width - 24.0f - goalSize.x, bottom.y + (bottom.height - GOAL_SIZE) / 2.0f }, GOAL_SIZE, TEXT_COLOR);
	}
} // namespace infrastructure::ui
