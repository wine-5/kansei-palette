#pragma once
#include "UiAction.h"
#include "game/flow/GameFlow.h"

namespace infrastructure::resource
{
	class Assets;
}

namespace infrastructure::ui
{
	/**
	 * @brief タイトル画面とエンディング画面の UI
	 * @details 初回は白黒の「かんせいのパレット」と「はじめる」。
	 *          エンディング後は文字が 1 文字ずつ色づき、「もういちど あそぶ」になる。
	 */
	class TitleScreen
	{
	public:
		/**
		 * @brief ボタンの操作を判定する
		 * @param flow ゲームの状態
		 * @return 操作結果
		 */
		UiAction update(const game::flow::GameFlow& flow);

		/**
		 * @brief 描く
		 * @param flow ゲームの状態
		 * @param assets アセット
		 */
		void draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const;
	};
} // namespace infrastructure::ui
