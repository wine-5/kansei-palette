#pragma once
#include "raylib.h"

namespace infrastructure::render
{
	class RestoreShader;
}

namespace infrastructure::debug
{
	/**
	 * @brief 色の復元シェーダーの確認用画面(F1 で表示を切り替える。F3 で世界の色をすべて戻した見た目にする)
	 * @details 色相を横に並べた帯を、復元度を変えて何段か描く。
	 *          ネイティブと Web で同じ見た目になるかを確かめるために使う。
	 */
	class ShaderPreview
	{
	public:
		/// 確認用の画像を作る(InitWindow の後に呼ぶ)
		void init();

		/// 画像を解放する
		void unload();

		/// F1 で確認画面、F3 で「すべての色が戻った見た目」を切り替える
		void update();

		/**
		 * @brief 表示中なら描く(BeginDrawing 〜 EndDrawing の中、最後に呼ぶ)
		 * @param shader 確認するシェーダー
		 */
		void draw(render::RestoreShader& shader) const;

		/// 表示中か
		bool isVisible() const { return m_isVisible; }

		/// F3: 世界の色をすべて戻した見た目にするか(ゲームの進行とは無関係に見た目だけ変える)
		bool isForcingFullRestore() const { return m_isForcingFullRestore; }

	private:
		Texture2D m_hueStrip{}; // 色相 0〜360 度を横に並べた画像
		bool m_isVisible{};
		bool m_isForcingFullRestore{};
	};
} // namespace infrastructure::debug
