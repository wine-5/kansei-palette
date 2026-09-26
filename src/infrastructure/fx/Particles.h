#pragma once
#include "raylib.h"
#include "game/data/Config.h"
#include <array>

namespace infrastructure::fx
{
	/**
	 * @brief 星くず(加算合成の光の粒)
	 * @details 上限 PARTICLE_MAX 粒を使い回す(足りなくなったら古い粒から上書きする)。
	 *          寿命 0.9〜1.9 秒、重力で落ち、地面(盤面の高さ)で弱く跳ね返る。
	 */
	class Particles
	{
	public:
		/// 粒の見た目(中心が明るくぼやけた円)を作る。InitWindow の後に呼ぶ
		void init();

		/// 粒の画像を解放する
		void unload();

		/**
		 * @brief 粒をまとめて打ち上げる
		 * @param position 出す位置(ワールド座標)
		 * @param count 粒の数
		 * @param colors 色の候補(ランダムに選ぶ)
		 * @param colorCount 色の候補の数
		 * @param speed 打ち上げの強さ(1.0 が標準)
		 */
		void spawnBurst(Vector3 position, int count, const Color* colors, int colorCount, float speed = 1.0f);

		/**
		 * @brief 粒を上からゆっくり降らせる(エンディング用)
		 * @param position 出す位置(ワールド座標)
		 * @param color 色
		 */
		void spawnFalling(Vector3 position, Color color);

		/**
		 * @brief 1 フレーム分進める
		 * @param dt 経過秒数
		 */
		void update(float dt);

		/**
		 * @brief 描く(BeginMode3D の中で呼ぶ。加算合成と深度の設定は内部で行う)
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
			float m_size{};
			float m_gravity{};
		};

		/// 次に使う粒(使っていない粒が無ければ一番古いもの)
		Particle& nextParticle();

		std::array<Particle, game::data::PARTICLE_MAX> m_particles{};
		int m_nextIndex{};
		Texture2D m_texture{};
	};
} // namespace infrastructure::fx
