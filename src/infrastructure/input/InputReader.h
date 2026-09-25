#pragma once
#include "raylib.h"
#include "game/flow/GameInput.h"
#include "infrastructure/ui/UiAction.h"

namespace infrastructure::input
{
	/**
	 * @brief raylib の入力(マウス・タッチ・キーボード)を game 層向けの GameInput に変換する
	 * @details マウス位置からレイを飛ばし、y = 0 の平面との交点からマスの行と列を求める。
	 *          raylib 6.0 では GetMouseRay は GetScreenToWorldRay に名前が変わっている。
	 *          タッチのタップは raylib がマウスの左クリックとして扱う。
	 */
	class InputReader
	{
	public:
		/**
		 * @brief このフレームの入力を読む
		 * @param camera 盤面を映しているカメラ
		 * @param uiAction UI ボタンの操作結果(ボタンの上ではマスのタップとして扱わない)
		 * @return game 層向けの入力
		 */
		game::flow::GameInput read(const Camera3D& camera, const ui::UiAction& uiAction) const;

	private:
		/**
		 * @brief 画面上の位置から盤面のマスを求める
		 * @param camera カメラ
		 * @param screenPos 画面上の位置
		 * @param outRow 行(盤面の外なら -1)
		 * @param outCol 列(盤面の外なら -1)
		 */
		static void pickCell(const Camera3D& camera, Vector2 screenPos, int& outRow, int& outCol);
	};
} // namespace infrastructure::input
