#pragma once
#include "GameInput.h"
#include "GamePhase.h"
#include "RestoreLevel.h"
#include <array>
#include "game/board/Board.h"
#include "game/data/Stages.h"
#include "game/event/GameEvent.h"
#include "game/hero/HeroState.h"

namespace game::flow
{
	/**
	 * @brief ゲーム全体の進行を管理する
	 * @details 進行状態・盤面・主人公・色の復元度を持ち、1 フレームごとに入力を受けて進める。
	 *          起きたことは GameEvent として記録し、infrastructure 層がそれを読んで音や演出を出す。
	 */
	class GameFlow
	{
	public:
		GameFlow();

		/**
		 * @brief 1 フレーム分進める
		 * @param dt 経過秒数
		 * @param input このフレームの操作
		 */
		void update(float dt, const GameInput& input);

		/// 現在の進行状態
		GamePhase getPhase() const { return m_phase; }

		/// 現在の進行状態になってからの秒数(クリア演出のタイムラインなどに使う)
		float getPhaseTime() const { return m_phaseTime; }

		/// 現在のステージ番号(0 始まり)
		int getStageIndex() const { return m_stageIndex; }

		/// 現在の盤面
		const board::Board& getBoard() const { return m_board; }

		/// 主人公の状態
		const hero::HeroState& getHero() const { return m_hero; }

		/// 現在の色の復元度(目標へ向かって少しずつ変化する)
		const RestoreLevel& getRestore() const { return m_restore; }

		/// せり上がった小物の数(ステージごと)
		int getRaisedPropCount(int stageIndex) const;

		/// 音が ON か
		bool isSoundOn() const { return m_isSoundOn; }

		/// エンディングを一度でも迎えたか(タイトルの表示を切り替える)
		bool hasReachedEnding() const { return m_hasReachedEnding; }

		/// このフレームに起きたこと(update のたびに作り直す)
		const event::GameEventList& getEvents() const { return m_events; }

	private:
		/// 進行状態を切り替える(経過時間を 0 に戻す)
		void changePhase(GamePhase phase);

		/// 指定したステージを初期状態で始める
		void startStage(int stageIndex);

		/// 指定したステージの盤面を初期状態で読み込む
		void loadBoard(int stageIndex);

		/// 起きたことを記録する
		void pushEvent(event::GameEventType type, int row = -1, int col = -1, int value = 0);

		/// 最初からやり直す(色と小物をリセットしてタイトルへ)
		void resetAll();

		void updateTitle(const GameInput& input);
		void updateStageIntro(const GameInput& input);
		void updatePlaying(const GameInput& input);
		void updateClearing();
		void updateClearCard(const GameInput& input);
		void updateEnding(const GameInput& input);

		GamePhase m_phase = GamePhase::Title;
		float m_phaseTime{};
		int m_stageIndex{};
		board::Board m_board;
		hero::HeroState m_hero;
		RestoreLevel m_restore;        // 現在の復元度
		RestoreLevel m_restoreTarget;  // 目標の復元度(ステージクリアで 1 になる)
		std::array<int, data::STAGES.size()> m_raisedPropCounts{}; // ステージごとのせり上がった小物の数
		bool m_isSoundOn{ true };
		bool m_hasReachedEnding{};
		event::GameEventList m_events;
	};
} // namespace game::flow
