#include "Board.h"

namespace game::board
{
	bool Board::isInside(int row, int col)
	{
		return row >= 0 && row < SIZE && col >= 0 && col < SIZE;
	}

	const Tile& Board::getTile(int row, int col) const
	{
		return m_tiles[row * SIZE + col];
	}

	void Board::setTile(int row, int col, const Tile& tile)
	{
		m_tiles[row * SIZE + col] = tile;
	}

	bool Board::rotate(int row, int col)
	{
		// TODO: 実装する(回せるタイルなら回転数を +1 して 0〜3 で循環させ、recalculatePower() を呼ぶ)
		(void)row;
		(void)col;
		return false;
	}

	void Board::recalculatePower()
	{
		// TODO: 実装する(全タイルの通電をリセット → 電源から幅優先探索 → m_isPowered と m_distance を設定)
	}

	int Board::countLitGoals() const
	{
		// TODO: 実装する
		return 0;
	}

	int Board::countGoals() const
	{
		// TODO: 実装する
		return 0;
	}

	bool Board::isCleared() const
	{
		// TODO: 実装する(ゴールが 1 つ以上あり、すべて点灯している)
		return false;
	}
} // namespace game::board
