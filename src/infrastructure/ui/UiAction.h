#pragma once

namespace infrastructure::ui
{
	/**
	 * @brief 1 フレーム分の UI ボタンの操作結果
	 * @details infrastructure::input::InputReader が game::flow::GameInput にまとめる。
	 */
	struct UiAction
	{
		bool m_isConfirmClicked{};     // 「はじめる」「つぎへ」「もういちど あそぶ」
		bool m_isResetClicked{};       // 「やりなおす」
		bool m_isSoundToggleClicked{}; // 「音」
		bool m_isPointerOverUi{};      // マウスがボタンの上にある(盤面のタップとして扱わない)
	};
} // namespace infrastructure::ui
