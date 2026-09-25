#pragma once
#include "HeroPose.h"

namespace game::hero
{
	/**
	 * @brief 主人公の状態(ポーズ・位置・タイマー)
	 * @details どのポーズを出すかだけを決める。どの画像を何 fps で出すかは infrastructure::render が決める。
	 */
	class HeroState
	{
	public:
		/**
		 * @brief 1 フレーム分進める(一時的なポーズの終了、考え込むまでのタイマー、入場の移動)
		 * @param dt 経過秒数
		 * @param hasInput このフレームに操作があったか
		 */
		void update(float dt, bool hasInput);

		/// ステージ開始: 画面の左外から立ち位置まで走って入ってくる
		void startRunIn();

		/// タイトルの立ち位置に置く
		void placeAtTitle();

		/// ロックタイルをタップした: 汗をかく
		void onTileBlocked();

		/// やりなおした: 目が回る
		void onReset();

		/// ステージをクリアした: 喜ぶ
		void onStageCleared();

		/// エンディング: 星を伴う喜び
		void onEnding();

		/// 現在のポーズ
		HeroPose getPose() const { return m_pose; }

		/// 現在のポーズになってからの秒数(アニメーションのコマ送りに使う)
		float getPoseTime() const { return m_poseTime; }

		/// 足元の X 座標(ワールド単位)
		float getX() const { return m_x; }

		/// 足元の Z 座標(ワールド単位)
		float getZ() const { return m_z; }

	private:
		HeroPose m_pose = HeroPose::Idle;
		float m_poseTime{};  // 現在のポーズの経過秒数
		float m_idleTimer{}; // 操作がない時間(考え込むまでのタイマー)
		float m_x{};
		float m_z{};
	};
} // namespace game::hero
