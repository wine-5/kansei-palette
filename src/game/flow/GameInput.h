#pragma once

namespace game::flow
{
	/**
	 * @brief 1 フレーム分のプレイヤーの操作
	 * @details raylib の入力を infrastructure::input::InputReader がこの形に変換して渡す。
	 *          game 層はマウス座標やキーコードを知らない。
	 */
	struct GameInput
	{
		int m_tappedRow{ -1 };   // タップしたマス(無ければ -1)
		int m_tappedCol{ -1 };
		int m_hoveredRow{ -1 };  // マウスが乗っているマス(無ければ -1)
		int m_hoveredCol{ -1 };
		bool m_isConfirmPressed{}; // 「はじめる」「つぎへ」「もういちど あそぶ」、Enter / Space
		bool m_isResetPressed{};   // 「やりなおす」、R キー
		bool m_isSoundTogglePressed{}; // 「音」ボタン
		bool m_hasAnyInput{};      // 何か操作があったか(考え込むまでのタイマーのリセット用)

		/// マスをタップしたか
		bool hasTap() const
		{
			return m_tappedRow >= 0 && m_tappedCol >= 0;
		}
	};
} // namespace game::flow
