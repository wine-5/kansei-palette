#pragma once

namespace game::flow
{
	/**
	 * @brief ゲームの進行状態(要件定義 3章「ゲームの流れ」)
	 */
	enum class GamePhase
	{
		Title,      // タイトル(白黒)。「はじめる」でステージ 1 へ
		StageIntro, // 主人公が走って入ってくる
		Playing,    // タイルを回して解く
		Clearing,   // クリア演出(約 2.7 秒)。回転入力を受け付けない
		ClearCard,  // クリアカード。「つぎへ」で次のステージへ
		Ending      // エンディング(色づいたタイトル)。「もういちど あそぶ」で最初から
	};
} // namespace game::flow
