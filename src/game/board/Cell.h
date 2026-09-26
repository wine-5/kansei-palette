#pragma once
#include "game/data/Config.h"

namespace game::board
{
	/**
	 * @brief 盤面のマスの位置(行・列)
	 */
	struct Cell
	{
		int m_row{};
		int m_col{};
	};

	/// 盤面の 1 辺のマス数(Board::SIZE と同じ)
	constexpr int BOARD_SIZE{ 5 };

	/**
	 * @brief マスの中心の X 座標(ワールド単位。盤面の中心が 0、右が +)
	 * @param col 列
	 * @return X 座標
	 */
	constexpr float cellCenterX(int col)
	{
		return (col - (BOARD_SIZE - 1) / 2.0f) * data::TILE_SIZE;
	}

	/**
	 * @brief マスの中心の Z 座標(ワールド単位。盤面の中心が 0、手前が +)
	 * @param row 行
	 * @return Z 座標
	 */
	constexpr float cellCenterZ(int row)
	{
		return (row - (BOARD_SIZE - 1) / 2.0f) * data::TILE_SIZE;
	}
} // namespace game::board
