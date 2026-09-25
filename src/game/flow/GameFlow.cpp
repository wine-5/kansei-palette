#include "GameFlow.h"
#include "game/board/StageParser.h"

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
		loadBoard(stageIndex);
		m_hero.startRunIn();
		pushEvent(event::GameEventType::StageStarted, -1, -1, stageIndex);
		changePhase(GamePhase::StageIntro);
	}

	void GameFlow::loadBoard(int stageIndex)
	{
		m_stageIndex = stageIndex;
		if (const auto board{ board::parseStage(data::STAGES[stageIndex].m_layout) })
			m_board = *board;
	}

	void GameFlow::pushEvent(event::GameEventType type, int row, int col, int value)
	{
		m_events.push_back(event::GameEvent{ type, row, col, value });
	}

	void GameFlow::resetAll()
	{
		// TODO: 実装する(色・小物を初期化する)
		// タイトルでも箱庭の上に盤面を見せるため、ステージ 1 の盤面を読み込んでおく
		loadBoard(0);
		changePhase(GamePhase::Title);
	}

	void GameFlow::updateTitle(const GameInput& input)
	{
		// TODO: 「はじめる」ボタンができたら、画面のクリックでは始めないようにする
		if (!input.m_isConfirmPressed && !input.m_isPointerPressed)
			return;
		pushEvent(event::GameEventType::GameStarted);
		startStage(0);
	}

	void GameFlow::updateStageIntro(const GameInput& input)
	{
		// 主人公が立ち位置に着いたら(走りのポーズが終わったら)遊べるようにする
		(void)input;
		if (m_hero.getPose() != hero::HeroPose::Run)
			changePhase(GamePhase::Playing);
	}

	void GameFlow::updatePlaying(const GameInput& input)
	{
		if (input.m_isResetPressed)
		{
			loadBoard(m_stageIndex);
			m_hero.onReset();
			pushEvent(event::GameEventType::StageReset);
			return;
		}

		if (!input.hasTap())
			return;

		const int row{ input.m_tappedRow };
		const int col{ input.m_tappedCol };
		const int litBefore{ m_board.countLitGoals() };
		if (!m_board.rotate(row, col))
		{
			// 回せないタイル。ロックタイルのときだけ主人公が汗をかく
			if (m_board.getTile(row, col).m_type == board::TileType::Locked)
				m_hero.onTileBlocked();
			pushEvent(event::GameEventType::TileBlocked, row, col);
			return;
		}
		pushEvent(event::GameEventType::TileRotated, row, col);

		const int litAfter{ m_board.countLitGoals() };
		if (litAfter > litBefore)
			pushEvent(event::GameEventType::GoalLit, -1, -1, litAfter);

		if (m_board.isCleared())
		{
			m_hero.onStageCleared();
			pushEvent(event::GameEventType::StageCleared, -1, -1, m_stageIndex);
			changePhase(GamePhase::Clearing);
		}
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
