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
	 * @brief ステージクリア後に画面下に出るカード
	 * @details 強調色の丸、「○○が もどった」、「あと N つの いろ」、「つぎへ」ボタン。
	 */
	class ClearCard
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
