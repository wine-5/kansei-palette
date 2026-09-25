#pragma once
#include "TileType.h"
#include "core/Direction.h"

namespace game::board
{
	/**
	 * @brief 盤面の 1 マス(ルール上の状態だけを持つ)
	 * @details 回転アニメーションや光の強さなど見た目の状態は infrastructure::render 側で持つ。
	 */
	struct Tile
	{
		TileType m_type = TileType::Blank;
		int m_rotation{};     // 時計回りの回転数(0〜3)。タップした瞬間に確定する
		bool m_isPowered{};   // 電気が届いているか
		int m_distance{ -1 }; // 電源からの距離(電源 = 0、届いていなければ -1)。色づきの遅延に使う

		/**
		 * @brief 現在の回転数を反映した接続方向を返す
		 * @return 接続方向
		 */
		core::DirectionMask connections() const
		{
			return core::rotateClockwise(baseConnections(m_type), m_rotation);
		}
	};
} // namespace game::board
