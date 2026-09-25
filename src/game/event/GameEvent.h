#pragma once
#include <vector>

namespace game::event
{
	/**
	 * @brief game 層で起きたことの種類
	 * @details game 層は音や演出を直接呼ばず、起きたことだけを記録する。
	 *          infrastructure 層(Audio・Effects)が毎フレームこれを読んで、音や演出を出す。
	 */
	enum class GameEventType
	{
		GameStarted,     // 「はじめる」が押された(音声の初期化にも使う)
		TileRotated,     // タイルを回した
		TileBlocked,     // 回せないタイルをタップした
		GoalLit,         // ゴールが点灯した
		StageCleared,    // すべてのゴールが点灯した
		ColorRestoring,  // 色の復元が始まった(クリアの 0.45 秒後)
		PropRaised,      // 小物が 1 つせり上がった
		StageStarted,    // 次のステージが始まった
		StageReset,      // やりなおした
		EndingStarted,   // エンディングが始まった
		SoundToggled     // 音の ON/OFF を切り替えた
	};

	/**
	 * @brief 起きたこと 1 件
	 */
	struct GameEvent
	{
		GameEventType m_type = GameEventType::TileRotated;
		int m_row{ -1 };   // 関係するマス(無ければ -1)
		int m_col{ -1 };
		int m_value{};     // 種類ごとの追加情報(GoalLit なら点灯数、PropRaised なら小物の番号など)
	};

	/// 1 フレーム分の出来事の一覧
	using GameEventList = std::vector<GameEvent>;
} // namespace game::event
