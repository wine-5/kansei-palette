#include "TileType.h"

namespace game::board
{
	core::DirectionMask baseConnections(TileType type)
	{
		// TODO: 実装する(要件定義 3章の表: 直線 N,S / 曲がり N,E / T字 N,E,W / 十字 全方向 / 電源 S / ゴール N / ロック N,S / 空き なし)
		(void)type;
		return 0;
	}

	bool isRotatable(TileType type)
	{
		// TODO: 実装する
		(void)type;
		return false;
	}

	std::optional<TileType> tileTypeFromSymbol(char symbol)
	{
		// TODO: 実装する
		(void)symbol;
		return std::nullopt;
	}
} // namespace game::board
