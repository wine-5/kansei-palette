#pragma once
#include "core/Direction.h"
#include <optional>

namespace game::board
{
	/**
	 * @brief タイルの種類(要件定義 3章「タイルの種類」)
	 */
	enum class TileType
	{
		Straight, // S: 直線
		Corner,   // C: 曲がり
		Tee,      // T: T字
		Cross,    // X: 十字
		Source,   // P: 電源
		Goal,     // G: ゴール
		Locked,   // L: ロック直線
		Blank     // B: 空き
	};

	/**
	 * @brief 基準の向き(回転数 0)での接続方向を返す
	 * @param type タイルの種類
	 * @return 接続方向
	 */
	core::DirectionMask baseConnections(TileType type);

	/**
	 * @brief プレイヤーが回せるタイルかどうか
	 * @param type タイルの種類
	 * @return 直線・曲がり・T字・十字なら true(電源・ゴール・ロック・空きは false)
	 */
	bool isRotatable(TileType type);

	/**
	 * @brief ステージデータの記号(S, C, T, X, P, G, L, B)からタイルの種類を求める
	 * @param symbol 記号 1 文字
	 * @return タイルの種類。不明な記号なら std::nullopt
	 */
	std::optional<TileType> tileTypeFromSymbol(char symbol);
} // namespace game::board
