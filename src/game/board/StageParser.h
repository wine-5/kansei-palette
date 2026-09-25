#pragma once
#include "Board.h"
#include <optional>

namespace game::board
{
	/**
	 * @brief ステージデータの文字列から盤面を作る
	 * @details 1 マスは 2 文字(タイル記号 + 初期の回転数 0〜3)。段ごとに改行、マスはスペース区切り。
	 *          例: "C2 B0 S1 B0 G2\n..."
	 * @param layout ステージデータの文字列
	 * @return 盤面。書式が不正なら std::nullopt
	 */
	std::optional<Board> parseStage(const char* layout);
} // namespace game::board
