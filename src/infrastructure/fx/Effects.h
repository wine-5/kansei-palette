#pragma once
#include "Particles.h"
#include "raylib.h"
#include "game/event/GameEvent.h"
#include "game/flow/GameFlow.h"
#include <array>

namespace infrastructure::fx
{
	/**
	 * @brief 演出をまとめて管理する(星くず・地面に広がる輪・白いフラッシュ・カメラの揺れ)
	 * @details GameEvent を読んで演出を始める。game 層は演出の存在を知らない。
	 */
	class Effects
	{
	public:
		/// 画像などを準備する(InitWindow の後に呼ぶ)
		void init();

		/// 解放する
		void unload();

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
		/**
		 * @brief 地面に広がって消えていく輪
		 */
		struct Ring
		{
			Vector3 m_center{};
			float m_maxRadius{};
			float m_time{};     // 経過秒数
			Color m_color{};
			bool m_isActive{};
		};

		void spawnRing(Vector3 center, float maxRadius, Color color);
		void startFlash(float alpha);
		void startShake(float strength);

		Particles m_particles;
		Texture2D m_ringTexture{}; // 輪の画像(縁だけが明るい円)
		Model m_ringModel{};       // 輪を地面に寝かせて描くための 1×1 の板
		std::array<Ring, 16> m_rings{};
		int m_nextRing{};
		float m_flashAlpha{};    // 白いフラッシュの不透明度(時間とともに 0 へ)
		float m_flashStart{};    // フラッシュを始めたときの不透明度
		float m_shakeStrength{}; // カメラの揺れの強さ(時間とともに 0 へ)
		float m_shakeTime{};
		Vector3 m_cameraShake{};
		bool m_isEndingRain{};   // エンディング中は色とりどりの星くずを降らせ続ける
		float m_rainTimer{};
	};
} // namespace infrastructure::fx
