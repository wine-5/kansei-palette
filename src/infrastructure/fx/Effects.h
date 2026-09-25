#pragma once
#include "Particles.h"
#include "raylib.h"
#include "game/event/GameEvent.h"
#include "game/flow/GameFlow.h"

namespace infrastructure::fx
{
	/**
	 * @brief 演出をまとめて管理する(星くず・輪・白いフラッシュ・カメラの揺れ)
	 * @details GameEvent を読んで演出を始める。game 層は演出の存在を知らない。
	 */
	class Effects
	{
	public:
		/**
		 * @brief このフレームに起きたことに応じて演出を始める
		 * @param events 起きたこと
		 * @param flow ゲームの状態(ゴールの位置やステージの色を知るため)
		 */
		void onEvents(const game::event::GameEventList& events, const game::flow::GameFlow& flow);

		/**
		 * @brief 1 フレーム分進める
		 * @param dt 経過秒数
		 */
		void update(float dt);

		/**
		 * @brief 3D 空間の演出を描く(BeginMode3D の中で呼ぶ)
		 * @param camera カメラ
		 */
		void drawWorld(const Camera3D& camera) const;

		/// 画面全体の演出を描く(フラッシュなど。UI より前に呼ぶ)
		void drawScreen() const;

		/// 現在のカメラの揺れ(WorldRenderer に渡す)
		Vector3 getCameraShake() const { return m_cameraShake; }

	private:
		Particles m_particles;
		float m_flashAlpha{};     // 白いフラッシュの不透明度
		float m_shakeStrength{};  // カメラの揺れの強さ
		Vector3 m_cameraShake{};
	};
} // namespace infrastructure::fx
