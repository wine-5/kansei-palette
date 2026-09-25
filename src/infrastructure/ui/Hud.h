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
	 * @brief プレイ中の HUD(要件定義 6章「HUD」)
	 * @details 左上: 「STAGE n / 3」、ステージ名、取り戻した色の丸 3 つ。
	 *          右上: 「やりなおす (R)」「音 ON/OFF」ボタン。
	 *          下: ヒント文と「ゴール 点灯数 / 全体」。
	 */
	class Hud
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
