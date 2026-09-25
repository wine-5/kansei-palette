#pragma once
#include "Tile.h"
#include <array>

namespace game::board
{
	/**
	 * @brief 5×5 の盤面。回転・通電判定・クリア判定を行う
	 * @details 描画には依存しない。raylib なしでテストできる(tests/BoardTest.cpp)。
	 */
	class Board
	{
	public:
		/// 盤面の 1 辺のマス数
		static constexpr int SIZE{ 5 };

		/**
		 * @brief 指定したマスが盤面の内側かどうか
		 * @param row 行(上から 0 始まり)
		 * @param col 列(左から 0 始まり)
		 * @return 内側なら true
		 */
		static bool isInside(int row, int col);

		/**
		 * @brief マスを取得する
		 * @param row 行
		 * @param col 列
		 * @return タイル
		 */
		const Tile& getTile(int row, int col) const;

		/**
		 * @brief マスを設定する(ステージ読み込み用)
		 * @param row 行
		 * @param col 列
		 * @param tile 設定するタイル
		 */
		void setTile(int row, int col, const Tile& tile);

		/**
		 * @brief タイルを時計回りに 90 度回し、通電を再計算する
		 * @param row 行
		 * @param col 列
		 * @return 回せたら true。回せないタイル(電源・ゴール・ロック・空き)なら false
		 */
		bool rotate(int row, int col);

		/**
		 * @brief 電源から幅優先探索で通電を再計算する
		 * @details 向かい合う辺の両方に接点があるときだけ電気が渡る(要件定義 3章「通電の判定」)。
		 */
		void recalculatePower();

		/// 点灯しているゴールの数
		int countLitGoals() const;

		/// 盤面上のゴールの総数
		int countGoals() const;

		/// すべてのゴールが点灯しているか
		bool isCleared() const;

	private:
		std::array<Tile, SIZE * SIZE> m_tiles{};
	};
} // namespace game::board
