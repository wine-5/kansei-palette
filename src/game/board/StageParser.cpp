#include "StageParser.h"
#include <cctype>

namespace game::board
{
	std::optional<Board> parseStage(const char* layout)
	{
		if (layout == nullptr)
			return std::nullopt;

		// 空白・改行を読み飛ばしながら「記号 + 回転数」の 2 文字ずつを 25 マス分読む
		Board board;
		const char* p{ layout };
		for (int i{}; i < Board::SIZE * Board::SIZE; ++i)
		{
			while (*p != '\0' && std::isspace(static_cast<unsigned char>(*p)))
				++p;

			const std::optional<TileType> type{ tileTypeFromSymbol(p[0]) };
			if (!type || p[1] < '0' || p[1] > '3')
				return std::nullopt;

			Tile tile{};
			tile.m_type = *type;
			tile.m_rotation = p[1] - '0';
			board.setTile(i / Board::SIZE, i % Board::SIZE, tile);
			p += 2;
		}

		// 余計なマスが続いていたら書式の誤り
		while (*p != '\0' && std::isspace(static_cast<unsigned char>(*p)))
			++p;
		if (*p != '\0')
			return std::nullopt;

		board.recalculatePower();
		return board;
	}
} // namespace game::board
