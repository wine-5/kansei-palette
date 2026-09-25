#include "GameFlow.h"

namespace game::flow
{
	GameFlow::GameFlow()
	{
		resetAll();
	}

	void GameFlow::update(float dt, const GameInput& input)
	{
		m_events.clear();
		m_phaseTime += dt;

		switch (m_phase)
		{
		case GamePhase::Title: updateTitle(input); break;
		case GamePhase::StageIntro: updateStageIntro(input); break;
		case GamePhase::Playing: updatePlaying(input); break;
		case GamePhase::Clearing: updateClearing(); break;
		case GamePhase::ClearCard: updateClearCard(input); break;
		case GamePhase::Ending: updateEnding(input); break;
		}

		m_hero.update(dt, input.m_hasAnyInput);
		// TODO: 実装する(m_restore を m_restoreTarget へ RESTORE_RATE で近づける。core::approachExp)
		// TODO: 実装する(どの状態でも「音」ボタンで m_isSoundOn を切り替え、SoundToggled を記録する)
	}

	int GameFlow::getRaisedPropCount(int stageIndex) const
	{
		return m_raisedPropCounts[stageIndex];
	}

	void GameFlow::changePhase(GamePhase phase)
	{
		m_phase = phase;
		m_phaseTime = 0.0f;
	}

	void GameFlow::startStage(int stageIndex)
	{
		// TODO: 実装する(data::STAGES[stageIndex] を board::parseStage で読み込み、主人公を入場させて StageIntro へ)
		m_stageIndex = stageIndex;
	}

	void GameFlow::resetAll()
	{
		// TODO: 実装する(色・小物・ステージ番号を初期化してタイトルへ)
		changePhase(GamePhase::Title);
	}

	void GameFlow::updateTitle(const GameInput& input)
	{
		// TODO: 実装する(決定で GameStarted を記録し、ステージ 1 を始める)
		(void)input;
	}

	void GameFlow::updateStageIntro(const GameInput& input)
	{
		// TODO: 実装する(主人公が立ち位置に着いたら Playing へ)
		(void)input;
	}

	void GameFlow::updatePlaying(const GameInput& input)
	{
		// TODO: 実装する
		// - タップ: m_board.rotate() → TileRotated / TileBlocked(ロックなら主人公が汗)
		// - 点灯数が増えたら GoalLit、全点灯で StageCleared → Clearing へ
		// - やりなおし: 盤面を初期状態に戻し StageReset(主人公は目が回る)
		(void)input;
	}

	void GameFlow::updateClearing()
	{
		// TODO: 実装する(クリア演出のタイムライン)
		// - CLEAR_RESTORE_START で ColorRestoring、その色の目標復元度を 1 にする
		// - CLEAR_PROP_INTERVAL ごとに PropRaised
		// - CLEAR_CARD_TIME で最終ステージ以外は ClearCard、最終ステージは ENDING_DELAY 後に Ending へ
	}

	void GameFlow::updateClearCard(const GameInput& input)
	{
		// TODO: 実装する(決定で次のステージを始める)
		(void)input;
	}

	void GameFlow::updateEnding(const GameInput& input)
	{
		// TODO: 実装する(決定で resetAll() してステージ 1 から)
		(void)input;
	}
} // namespace game::flow
