#pragma once
#include "raylib.h"
#include "game/flow/RestoreLevel.h"

namespace infrastructure::render
{
	/**
	 * @brief 色の復元シェーダー(要件定義 5章「色の復元」)
	 * @details ピクセルの色相で判定し、まだ戻っていない色相は輝度だけのグレーにする。
	 *
	 *          【Web の落とし穴: シェーダーのバージョン】
	 *          Web(WebGL1)は GLSL ES 100、ネイティブ(OpenGL 3.3)は GLSL 330 が必要。
	 *          本体は 1 つにして、先頭の #version と精度指定だけを付け替えて LoadShaderFromMemory で読み込む。
	 *
	 *          【raylib の落とし穴: uniform と描画のまとめ処理】
	 *          raylib は DrawTexture / DrawBillboard などをまとめて後から GPU に送るため、
	 *          描画の合間に SetShaderValue で値を変えても、まとめて描かれた全体に最後の値が使われる。
	 *          タイルごとの通電度は uniform ではなく、描画色(tint)のアルファに入れて渡す。
	 */
	class RestoreShader
	{
	public:
		/**
		 * @brief シェーダーを読み込む
		 * @return 読み込めたら true(失敗したら企画書 8章の代替案に切り替える)
		 */
		bool load();

		/// シェーダーを解放する
		void unload();

		/**
		 * @brief 世界全体の復元度を設定する(フレームに 1 回、描画の前に呼ぶ)
		 * @param level 復元度
		 */
		void setRestoreLevel(const game::flow::RestoreLevel& level);

		/// このシェーダーで描き始める(BeginShaderMode)
		void begin() const;

		/// このシェーダーでの描画を終える(EndShaderMode)
		void end() const;

		/// 読み込めているか
		bool isLoaded() const { return m_isLoaded; }

	private:
		Shader m_shader{};
		int m_restoreLocation{ -1 }; // uniform vec3 restore(r, b, g)の場所
		bool m_isLoaded{};
	};
} // namespace infrastructure::render
