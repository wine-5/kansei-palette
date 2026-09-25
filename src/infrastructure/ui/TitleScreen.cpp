#include "TitleScreen.h"
#include "UiText.h"
#include "infrastructure/resource/Assets.h"
#include "game/data/Config.h"

namespace infrastructure::ui
{
	UiAction TitleScreen::update(const game::flow::GameFlow& flow)
	{
		// TODO: 実装する(「はじめる」/「もういちど あそぶ」ボタンの判定)
		(void)flow;
		return UiAction{};
	}

	void TitleScreen::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		// TODO: 実装する(サブタイトル、キャプション、ボタン、エンディング後の色づき)
		(void)flow;

		// 仮: 日本語フォントが読み込めているかの確認用にタイトルだけ出す
		const Font& font{ assets.getUiFont() };
		constexpr float size{ 64.0f };
		const Vector2 textSize{ MeasureTextEx(font, TEXT_TITLE, size, 1.0f) };
		DrawTextEx(font, TEXT_TITLE, Vector2{ (game::data::SCREEN_WIDTH - textSize.x) / 2.0f, 120.0f }, size, 1.0f, DARKGRAY);
	}
} // namespace infrastructure::ui
