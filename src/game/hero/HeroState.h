#pragma once
#include "HeroPose.h"
#include <vector>

namespace game::hero
{
	/**
	 * @brief 歩く道筋の 1 点(ワールド座標)
	 */
	struct Waypoint
	{
		float m_x{};
		float m_z{};
	};

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

		/**
		 * @brief ステージ開始: 画面の左外から立ち位置まで走って入ってくる
		 * @param x 立ち位置の X 座標(ワールド単位)
		 * @param z 立ち位置の Z 座標(ワールド単位。走っている間もこの奥行きのまま進む)
		 */
		void startRunIn(float x, float z);

		/// タイトルの立ち位置に置く
		void placeAtTitle();

		/// ロックタイルをタップした: 汗をかく
		void onTileBlocked();

		/// やりなおした: 目が回る
		void onReset();

		/**
		 * @brief 道筋に沿って歩き始める(着いたら待機のポーズになる)
		 * @param path 通る点の並び(最後がゴール)
		 */
		void startWalk(const std::vector<Waypoint>& path);

		/// 歩いている途中か
		bool isWalking() const { return m_pose == HeroPose::Walk; }

		/// 左を向いているか(画像は右向きなので、左へ歩くときは左右反転して描く)
		bool isFacingLeft() const { return m_isFacingLeft; }

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
		/// ポーズを切り替える(経過時間を 0 に戻す)
		void changePose(HeroPose pose);

		/// 道筋に沿って進める
		void updateWalk(float dt);

		HeroPose m_pose = HeroPose::Idle;
		float m_poseTime{};  // 現在のポーズの経過秒数
		float m_idleTimer{}; // 操作がない時間(考え込むまでのタイマー)
		float m_thinkInterval{ 14.0f }; // 次に考え込むまでの秒数(1 回目は長く、2 回目以降は短い)
		bool m_canThink{};   // 考え込んでよいか(ステージ中だけ)
		bool m_isFacingLeft{};
		std::vector<Waypoint> m_path; // 歩く道筋
		size_t m_pathIndex{};         // 次に向かう点
		float m_x{};
		float m_z{};
		float m_runTargetX{}; // 入場で走っていく先の X 座標
	};
} // namespace game::hero
