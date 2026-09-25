#pragma once

namespace core
{
	/**
	 * @brief 目標値へバネのように近づく値
	 * @details タイルの回転(強さ 260、減衰 23)や小物のせり上がり(強さ 190、減衰 13)で使う。
	 *          少し行き過ぎてから戻る動きになる(要件定義 5章「タイルの動き」)。
	 */
	struct Spring
	{
		float m_value{};    // 現在値
		float m_velocity{}; // 現在の速度
		float m_target{};   // 目標値

		/**
		 * @brief 1 フレーム分進める
		 * @param dt 経過秒数
		 * @param stiffness バネの強さ
		 * @param damping 減衰
		 */
		void update(float dt, float stiffness, float damping)
		{
			// TODO: 実装する(加速度 = (目標 - 現在) × 強さ - 速度 × 減衰)
			(void)dt;
			(void)stiffness;
			(void)damping;
		}

		/**
		 * @brief 目標値と現在値を同時に設定し、動きを止める
		 * @param value 設定する値
		 */
		void snapTo(float value)
		{
			m_value = value;
			m_target = value;
			m_velocity = 0.0f;
		}
	};
} // namespace core
