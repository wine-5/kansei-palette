#pragma once
#include "raylib.h"

namespace infrastructure::ui
{
	/**
	 * @brief カプセル型のボタン(押しているときは 3px 沈む。要件定義 6章「強調色とフォント」)
	 */
	class Button
	{
	public:
		/**
		 * @brief 押されたかを判定する(マウスのクリック・タッチのタップ)
		 * @param bounds ボタンの矩形(画面座標)
		 * @return このフレームに押されたら true
		 */
		static bool isClicked(Rectangle bounds);

		/**
		 * @brief マウスがボタンの上にあるか
		 * @param bounds ボタンの矩形
		 * @return 上にあれば true
		 */
		static bool isHovered(Rectangle bounds);

		/**
		 * @brief ボタンを描く
		 * @param bounds ボタンの矩形
		 * @param label 表示する文字
		 * @param font フォント
		 * @param accent 強調色
		 */
		static void draw(Rectangle bounds, const char* label, const Font& font, Color accent);
	};
} // namespace infrastructure::ui
