#pragma once
#include "raylib.h"
#include "game/flow/RestoreLevel.h"

namespace infrastructure::render
{
	/**
	 * @brief 色の復元シェーダー
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
	 *
	 *          【このシェーダーでの tint のアルファの意味】
	 *          不透明度ではなく「世界の復元度に従う度合い」(255 = 世界に従う、0 = 完全に彩色)。
	 *          WHITE のまま描けば世界の復元度に従う。通電したタイルは makeTint() で作った色で描く。
	 *          透明度は画像のアルファだけが使われる。
	 */
	class RestoreShader
	{
	public:
		/**
		 * @brief シェーダーを読み込む
		 * @return 読み込めたら true(失敗したら、グレーと彩色のクロスフェードに切り替える)
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

		/// シェーダー本体(Model の material に設定するときに使う)
		const Shader& getShader() const { return m_shader; }

		/**
		 * @brief このシェーダーで描くときの描画色を作る
		 * @param base 色味(通常は WHITE)
		 * @param selfRestore 世界の復元度に関係なく色を付ける度合い(0〜1。通電度を渡す)
		 * @return DrawTexture などに渡す tint
		 */
		static Color makeTint(Color base, float selfRestore);

	private:
		Shader m_shader{};
		int m_restoreLocation{ -1 }; // uniform vec3 restore(r, b, g)の場所
		bool m_isLoaded{};
	};
} // namespace infrastructure::render
