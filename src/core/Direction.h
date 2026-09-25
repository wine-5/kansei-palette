#pragma once
#include <cstdint>

namespace core
{
	/**
	 * @brief 盤面上の方向
	 * @details タイルの接続方向は、この値を OR した 4 ビットで表す(要件定義 3章「回転のルール」)。
	 *          時計回りに 1 回転すると、各ビットが North → East → South → West → North の順に移る。
	 */
	enum class Direction : std::uint8_t
	{
		North = 1,
		East = 2,
		South = 4,
		West = 8
	};

	/// 接続方向の組み合わせ(Direction を OR した 4 ビット)
	using DirectionMask = std::uint8_t;

	/// 全方向(十字タイル)
	constexpr DirectionMask DIRECTION_ALL{ 0b1111 };

	/**
	 * @brief 方向をビットに変換する
	 * @param dir 方向
	 * @return dir だけが立った DirectionMask
	 */
	constexpr DirectionMask toMask(Direction dir)
	{
		return static_cast<DirectionMask>(dir);
	}

	/**
	 * @brief 接続方向を時計回りに回転させる
	 * @param mask 回転前の接続方向
	 * @param turns 時計回りに 90 度ずつ回す回数(0〜3)
	 * @return 回転後の接続方向
	 */
	constexpr DirectionMask rotateClockwise(DirectionMask mask, int turns)
	{
		// TODO: 実装する(ビットを左に turns 回ずらし、あふれた分を下位へ戻す)
		(void)turns;
		return mask;
	}

	/**
	 * @brief 反対の方向を返す
	 * @param dir 方向
	 * @return 反対の方向(North なら South)
	 */
	constexpr Direction opposite(Direction dir)
	{
		// TODO: 実装する
		return dir;
	}
} // namespace core
