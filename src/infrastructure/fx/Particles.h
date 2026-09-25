#pragma once
#include "raylib.h"
#include "game/data/Config.h"
#include <array>

namespace infrastructure::fx
{
	/**
	 * @brief 星くず(加算合成の点の集まり)
	 * @details 上限 PARTICLE_MAX 粒を使い回す。寿命 0.9〜1.9 秒、重力あり、地面でバウンドする。
	 */
	class Particles
	{
	public:
		/**
		 * @brief 粒をまとめて出す
		 * @param position 出す位置(ワールド座標)
		 * @param count 粒の数
		 * @param color 色
		 */
		void spawnBurst(Vector3 position, int count, Color color);

		/**
		 * @brief 1 フレーム分進める
		 * @param dt 経過秒数
		 */
		void update(float dt);

		/**
		 * @brief 描く(BeginMode3D の中で呼ぶ。加算合成は内部で行う)
		 * @param camera カメラ
		 */
		void draw(const Camera3D& camera) const;

	private:
		struct Particle
		{
			Vector3 m_position{};
			Vector3 m_velocity{};
			Color m_color{};
			float m_life{};    // 残り寿命(0 以下なら未使用)
			float m_maxLife{};
		};

		std::array<Particle, game::data::PARTICLE_MAX> m_particles{};
		int m_nextIndex{}; // 次に使う粒(古いものから上書きする)
	};
} // namespace infrastructure::fx
