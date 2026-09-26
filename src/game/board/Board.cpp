#include "Board.h"
#include <queue>

namespace
{
	/**
	 * @brief 隣のマスへの移動量と、その方向
	 */
	struct Neighbor
	{
		int m_rowOffset{};
		int m_colOffset{};
		core::Direction m_direction = core::Direction::North;
	};

	constexpr Neighbor NEIGHBORS[]{
		{ -1, 0, core::Direction::North },
		{ 0, 1, core::Direction::East },
		{ 1, 0, core::Direction::South },
		{ 0, -1, core::Direction::West },
	};
} // namespace

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
		if (!isInside(row, col))
			return false;

		Tile& tile{ m_tiles[row * SIZE + col] };
		if (!isRotatable(tile.m_type))
			return false;

		tile.m_rotation = (tile.m_rotation + 1) % 4;
		recalculatePower();
		return true;
	}

	void Board::recalculatePower()
	{
		std::queue<int> queue; // 盤面の番号(row * SIZE + col)
		for (int i{}; i < SIZE * SIZE; ++i)
		{
			Tile& tile{ m_tiles[i] };
			const bool isSource{ tile.m_type == TileType::Source };
			tile.m_isPowered = isSource;
			tile.m_distance = isSource ? 0 : -1;
			if (isSource)
				queue.push(i);
		}

		// 電源から幅優先探索。向かい合う辺の両方に接点があるときだけ電気が渡る
		while (!queue.empty())
		{
			const int index{ queue.front() };
			queue.pop();
			const int row{ index / SIZE };
			const int col{ index % SIZE };
			const Tile& from{ m_tiles[index] };
			const core::DirectionMask fromConnections{ from.connections() };

			for (const Neighbor& neighbor : NEIGHBORS)
			{
				const int nextRow{ row + neighbor.m_rowOffset };
				const int nextCol{ col + neighbor.m_colOffset };
				if (!isInside(nextRow, nextCol))
					continue;

				Tile& to{ m_tiles[nextRow * SIZE + nextCol] };
				if (to.m_isPowered)
					continue;

				const bool fromHasContact{ (fromConnections & core::toMask(neighbor.m_direction)) != 0 };
				const bool toHasContact{ (to.connections() & core::toMask(core::opposite(neighbor.m_direction))) != 0 };
				if (!fromHasContact || !toHasContact)
					continue;

				to.m_isPowered = true;
				to.m_distance = from.m_distance + 1;
				queue.push(nextRow * SIZE + nextCol);
			}
		}
	}

	int Board::countLitGoals() const
	{
		int count{};
		for (const Tile& tile : m_tiles)
		{
			if (tile.m_type == TileType::Goal && tile.m_isPowered)
				++count;
		}
		return count;
	}

	int Board::countGoals() const
	{
		int count{};
		for (const Tile& tile : m_tiles)
		{
			if (tile.m_type == TileType::Goal)
				++count;
		}
		return count;
	}

	bool Board::findFarthestLitGoal(Cell& outGoal) const
	{
		int farthest{ -1 };
		for (int row{}; row < SIZE; ++row)
		{
			for (int col{}; col < SIZE; ++col)
			{
				const Tile& tile{ getTile(row, col) };
				if (tile.m_type == TileType::Goal && tile.m_isPowered && tile.m_distance > farthest)
				{
					farthest = tile.m_distance;
					outGoal = Cell{ row, col };
				}
			}
		}
		return farthest >= 0;
	}

	std::vector<Cell> Board::tracePathFromSource(Cell target) const
	{
		if (!isInside(target.m_row, target.m_col) || !getTile(target.m_row, target.m_col).m_isPowered)
			return {};

		// 行き先から、距離が 1 小さく、つながっている隣のマスへと電源まで戻る
		std::vector<Cell> path{ target };
		Cell current{ target };
		while (getTile(current.m_row, current.m_col).m_distance > 0)
		{
			const Tile& tile{ getTile(current.m_row, current.m_col) };
			bool isFound{};
			for (const Neighbor& neighbor : NEIGHBORS)
			{
				const int row{ current.m_row + neighbor.m_rowOffset };
				const int col{ current.m_col + neighbor.m_colOffset };
				if (!isInside(row, col))
					continue;
				const Tile& next{ getTile(row, col) };
				const bool isConnected{ (tile.connections() & core::toMask(neighbor.m_direction)) != 0
					&& (next.connections() & core::toMask(core::opposite(neighbor.m_direction))) != 0 };
				if (isConnected && next.m_isPowered && next.m_distance == tile.m_distance - 1)
				{
					current = Cell{ row, col };
					path.push_back(current);
					isFound = true;
					break;
				}
			}
			if (!isFound)
				return {};
		}
		return std::vector<Cell>(path.rbegin(), path.rend());
	}

	bool Board::isCleared() const
	{
		const int goals{ countGoals() };
		return goals > 0 && countLitGoals() == goals;
	}
} // namespace game::board
