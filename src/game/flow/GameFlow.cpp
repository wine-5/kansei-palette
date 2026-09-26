#include "GameFlow.h"
#include "core/Easing.h"
#include "game/board/StageParser.h"
#include "game/data/Config.h"
#include <algorithm>
#include <cmath>

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

		// 「音」ボタンはどの状態でも効く
		if (input.m_isSoundTogglePressed)
		{
			m_isSoundOn = !m_isSoundOn;
			pushEvent(event::GameEventType::SoundToggled, -1, -1, m_isSoundOn ? 1 : 0);
		}

		m_hero.update(dt, input.m_hasAnyInput);

		// 色の復元度は、目標へ少しずつ近づける(約 1.4 秒で 9 割)
		m_restore.m_red = core::approachExp(m_restore.m_red, m_restoreTarget.m_red, data::RESTORE_RATE, dt);
		m_restore.m_blue = core::approachExp(m_restore.m_blue, m_restoreTarget.m_blue, data::RESTORE_RATE, dt);
		m_restore.m_yellowGreen = core::approachExp(m_restore.m_yellowGreen, m_restoreTarget.m_yellowGreen, data::RESTORE_RATE, dt);
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
		m_restore = RestoreLevel{};
		m_restoreTarget = RestoreLevel{};
		m_raisedPropCounts.fill(0);
		m_hasReachedEnding = false;
		// タイトルでも箱庭の上に盤面を見せるため、ステージ 1 の盤面を読み込んでおく
		loadBoard(0);
		m_hero.placeAtTitle();
		changePhase(GamePhase::Title);
	}

	void GameFlow::updateTitle(const GameInput& input)
	{
		if (!input.m_isConfirmPressed)
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
			m_hasRestoreStarted = false;
			changePhase(GamePhase::Clearing);
		}
	}

	void GameFlow::updateClearing()
	{
		const data::StageDefinition& stage{ data::STAGES[m_stageIndex] };

		// 少し間をおいて、そのステージの色を世界に戻す
		if (!m_hasRestoreStarted && m_phaseTime >= data::CLEAR_RESTORE_START)
		{
			m_hasRestoreStarted = true;
			switch (stage.m_hue)
			{
			case data::HueBand::Red: m_restoreTarget.m_red = 1.0f; break;
			case data::HueBand::Blue: m_restoreTarget.m_blue = 1.0f; break;
			case data::HueBand::YellowGreen: m_restoreTarget.m_yellowGreen = 1.0f; break;
			}
			pushEvent(event::GameEventType::ColorRestoring, -1, -1, static_cast<int>(stage.m_hue));
		}

		// 色が戻り始めてから、小物を一定の間隔で 1 つずつせり上げる
		if (m_hasRestoreStarted)
		{
			const float elapsed{ m_phaseTime - data::CLEAR_RESTORE_START };
			const int shouldRaise{ std::min(stage.m_propCount, static_cast<int>(std::floor(elapsed / data::CLEAR_PROP_INTERVAL)) + 1) };
			int& raised{ m_raisedPropCounts[m_stageIndex] };
			while (raised < shouldRaise)
			{
				pushEvent(event::GameEventType::PropRaised, -1, -1, raised);
				++raised;
			}
		}

		const bool isLastStage{ m_stageIndex + 1 >= static_cast<int>(data::STAGES.size()) };
		if (!isLastStage && m_phaseTime >= data::CLEAR_CARD_TIME)
		{
			changePhase(GamePhase::ClearCard);
		}
		else if (isLastStage && m_phaseTime >= data::CLEAR_CARD_TIME + data::ENDING_DELAY)
		{
			m_hasReachedEnding = true;
			m_hero.onEnding();
			pushEvent(event::GameEventType::EndingStarted);
			changePhase(GamePhase::Ending);
		}
	}

	void GameFlow::updateClearCard(const GameInput& input)
	{
		if (input.m_isConfirmPressed)
			startStage(m_stageIndex + 1);
	}

	void GameFlow::updateEnding(const GameInput& input)
	{
		// 演出を見てもらうため、エンディングに入ってすぐのクリックは受け付けない
		constexpr float INPUT_DELAY{ 1.5f };
		if (m_phaseTime < INPUT_DELAY)
			return;
		if (!input.m_isConfirmPressed)
			return;
		// 色と小物をリセットして、ステージ 1 から
		resetAll();
		startStage(0);
	}
} // namespace game::flow
