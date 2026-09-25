#pragma once

namespace core
{
	/**
	 * @brief 目標値へバネのように近づく値
	 * @details タイルの回転(強さ 260、減衰 23)や小物のせり上がり(強さ 190、減衰 13)で使う。
	 *          少し行き過ぎてから戻る動きになる。
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
			// 加速度 = (目標 - 現在) × 強さ - 速度 × 減衰。速度を先に更新する(半陰的オイラー法で安定させる)
			const float acceleration{ (m_target - m_value) * stiffness - m_velocity * damping };
			m_velocity += acceleration * dt;
			m_value += m_velocity * dt;
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
