#pragma once

namespace game::hero
{
	/**
	 * @brief 主人公のポーズ(要件定義 3章「主人公の反応」)
	 */
	enum class HeroPose
	{
		Idle,    // 待機(少し呼吸する)
		Run,     // 走り(ステージ開始の入場)
		Sweat,   // 汗(ロックタイルをタップ)
		Think,   // 考え込む(しばらく操作がない)
		Dizzy,   // 目が回る(やりなおし)
		Cheer,   // 喜び(クリア演出)
		JoyStar  // 星を伴う喜び(エンディング)
	};
} // namespace game::hero
