#pragma once

namespace core
{
	/**
	 * @brief 目標値へ指数的に近づける
	 * @details 色の復元度(毎秒 1.7)や通電度(立ち上がり毎秒 9、低下毎秒 16)の変化に使う。
	 *          フレームレートが変わっても同じ速さで近づくよう、dt から係数を求める。
	 * @param current 現在値
	 * @param target 目標値
	 * @param rate 1 秒あたりの近づく割合
	 * @param dt 経過秒数
	 * @return 近づけた後の値
	 */
	inline float approachExp(float current, float target, float rate, float dt)
	{
		// TODO: 実装する(current + (target - current) × (1 - exp(-rate × dt)))
		(void)target;
		(void)rate;
		(void)dt;
		return current;
	}

	/**
	 * @brief 0〜1 の値を、終わりがゆっくりになる曲線に変換する
	 * @param t 0〜1 の進み具合
	 * @return 変換後の値(0〜1)
	 */
	inline float easeOutCubic(float t)
	{
		// TODO: 実装する
		return t;
	}
} // namespace core
