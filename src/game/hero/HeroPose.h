#pragma once

namespace game::hero
{
	/**
	 * @brief 主人公のポーズ
	 */
	enum class HeroPose
	{
		Idle,    // 待機(少し呼吸する)
		Run,     // 走り(ステージ開始の入場)
		Sweat,   // 汗(ロックタイルをタップ)
		Think,   // 考え込む(しばらく操作がない)
		Dizzy,   // 目が回る(やりなおし)
		Cheer,   // 喜び(クリア演出)
		JoyStar, // 星を伴う喜び(エンディング)
		Walk     // つながった道を歩いてゴールへ向かう
	};
} // namespace game::hero
