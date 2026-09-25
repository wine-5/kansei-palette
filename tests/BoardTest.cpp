// 盤面ロジックのテスト(raylib なしで動く)。
// 実行: cmake --build build --target KanseiTests && build\Debug\KanseiTests.exe
// 確認すること:
//   - 3 ステージとも、初期状態ではクリアしていない
//   - 解答表どおりに回すとクリアできる
//   - GameFlow でタイトルから始め、タップでステージ 1 をクリアできる
//   - 3 ステージを通してエンディングまで進み、もういちど遊ぶと最初に戻る
#include "game/board/StageParser.h"
#include "game/data/Stages.h"
#include "game/data/Config.h"
#include "game/flow/GameFlow.h"
#include <algorithm>
#include <cstdio>
#include <vector>

namespace
{
	/**
	 * @brief 解答の 1 手(このマスをこの回転数にする)
	 */
	struct SolutionStep
	{
		int m_row{};
		int m_col{};
		int m_rotation{};
	};

	int g_failCount{};

	void check(bool condition, const char* message)
	{
		std::printf("[%s] %s\n", condition ? " OK " : "FAIL", message);
		if (!condition)
			++g_failCount;
	}

	/// 各ステージの解答(十字タイルは向きが任意なので含めない)
	const std::vector<std::vector<SolutionStep>> SOLUTIONS{
		{ { 4, 1, 1 }, { 4, 2, 3 }, { 3, 2, 0 }, { 2, 2, 1 }, { 2, 3, 1 }, { 2, 4, 3 }, { 1, 4, 0 } },
		{ { 2, 2, 2 }, { 2, 1, 1 }, { 2, 0, 0 }, { 2, 3, 1 } },
		{ { 0, 1, 1 }, { 0, 2, 2 }, { 3, 2, 0 }, { 1, 3, 1 }, { 1, 4, 2 }, { 2, 4, 0 }, { 1, 1, 1 }, { 2, 1, 0 }, { 3, 1, 3 } },
	};

	void testStage(size_t stageIndex)
	{
		std::printf("--- stage %zu ---\n", stageIndex + 1);
		auto board{ game::board::parseStage(game::data::STAGES[stageIndex].m_layout) };
		check(board.has_value(), "ステージデータを読み込める");
		if (!board)
			return;

		check(board->countGoals() > 0, "ゴールがある");
		check(!board->isCleared(), "初期状態ではクリアしていない");

		// 電源・ゴール・ロック・空きは回らない
		bool areFixedTilesStable{ true };
		for (int row{}; row < game::board::Board::SIZE; ++row)
		{
			for (int col{}; col < game::board::Board::SIZE; ++col)
			{
				const auto& tile{ board->getTile(row, col) };
				if (game::board::isRotatable(tile.m_type))
					continue;
				const int before{ tile.m_rotation };
				if (board->rotate(row, col) || board->getTile(row, col).m_rotation != before)
					areFixedTilesStable = false;
			}
		}
		check(areFixedTilesStable, "回せないタイルは回らない");

		// 目標の回転数になるまで回す
		for (const auto& step : SOLUTIONS[stageIndex])
		{
			for (int i{}; i < 4 && board->getTile(step.m_row, step.m_col).m_rotation != step.m_rotation; ++i)
				board->rotate(step.m_row, step.m_col);
		}
		check(board->isCleared(), "解答どおりに回すとクリアできる");
	}

	bool hasEvent(const game::flow::GameFlow& flow, game::event::GameEventType type)
	{
		const auto& events{ flow.getEvents() };
		return std::any_of(events.begin(), events.end(), [type](const game::event::GameEvent& e) { return e.m_type == type; });
	}

	void testFlowStage1()
	{
		std::printf("--- flow: stage 1 ---\n");
		constexpr float DT{ 1.0f / 60.0f };
		game::flow::GameFlow flow;
		check(flow.getPhase() == game::flow::GamePhase::Title, "タイトルから始まる");

		game::flow::GameInput confirm{};
		confirm.m_isConfirmPressed = true;
		flow.update(DT, confirm);
		check(hasEvent(flow, game::event::GameEventType::GameStarted), "決定で GameStarted が記録される");

		// 主人公の入場が終わるまで進める
		for (int i{}; i < 600 && flow.getPhase() != game::flow::GamePhase::Playing; ++i)
			flow.update(DT, game::flow::GameInput{});
		check(flow.getPhase() == game::flow::GamePhase::Playing, "入場が終わると遊べる状態になる");

		// 回せないタイル(電源)をタップしても盤面は変わらない
		game::flow::GameInput tapSource{};
		tapSource.m_tappedRow = 4;
		tapSource.m_tappedCol = 0;
		flow.update(DT, tapSource);
		check(hasEvent(flow, game::event::GameEventType::TileBlocked), "電源をタップすると TileBlocked が記録される");

		bool isCleared{};
		for (const auto& step : SOLUTIONS[0])
		{
			for (int i{}; i < 4 && flow.getBoard().getTile(step.m_row, step.m_col).m_rotation != step.m_rotation; ++i)
			{
				game::flow::GameInput tap{};
				tap.m_tappedRow = step.m_row;
				tap.m_tappedCol = step.m_col;
				flow.update(DT, tap);
				isCleared = isCleared || hasEvent(flow, game::event::GameEventType::StageCleared);
			}
		}
		check(isCleared, "解答どおりにタップすると StageCleared が記録される");
		check(flow.getPhase() == game::flow::GamePhase::Clearing, "クリア演出に進む");

		// クリア演出中はタップしても回らない
		const int before{ flow.getBoard().getTile(0, 0).m_rotation };
		game::flow::GameInput tap{};
		tap.m_tappedRow = 0;
		tap.m_tappedCol = 0;
		flow.update(DT, tap);
		check(flow.getBoard().getTile(0, 0).m_rotation == before, "クリア演出中はタイルが回らない");
	}

	/// 指定した秒数だけ、何も操作せずに進める
	void advance(game::flow::GameFlow& flow, float seconds)
	{
		constexpr float DT{ 1.0f / 60.0f };
		for (float t{}; t < seconds; t += DT)
			flow.update(DT, game::flow::GameInput{});
	}

	/// 解答どおりにタップしてステージをクリアする
	void solveCurrentStage(game::flow::GameFlow& flow)
	{
		for (const auto& step : SOLUTIONS[flow.getStageIndex()])
		{
			for (int i{}; i < 4 && flow.getBoard().getTile(step.m_row, step.m_col).m_rotation != step.m_rotation; ++i)
			{
				game::flow::GameInput tap{};
				tap.m_tappedRow = step.m_row;
				tap.m_tappedCol = step.m_col;
				flow.update(1.0f / 60.0f, tap);
			}
		}
	}

	void testFullPlaythrough()
	{
		std::printf("--- flow: 3 stages to ending ---\n");
		game::flow::GameFlow flow;
		game::flow::GameInput confirm{};
		confirm.m_isConfirmPressed = true;
		flow.update(1.0f / 60.0f, confirm);

		for (int stage{}; stage < static_cast<int>(game::data::STAGES.size()); ++stage)
		{
			advance(flow, 0.1f);
			check(flow.getStageIndex() == stage && flow.getPhase() == game::flow::GamePhase::Playing, "ステージが始まって遊べる状態になる");
			solveCurrentStage(flow);
			check(flow.getPhase() == game::flow::GamePhase::Clearing, "解答どおりに回すとクリア演出に進む");

			advance(flow, game::data::CLEAR_CARD_TIME + 0.1f);
			check(flow.getRaisedPropCount(stage) == game::data::STAGES[stage].m_propCount, "クリア演出で、そのステージの小物がすべてせり上がる");

			const bool isLast{ stage + 1 == static_cast<int>(game::data::STAGES.size()) };
			if (!isLast)
			{
				check(flow.getPhase() == game::flow::GamePhase::ClearCard, "クリアカードが表示される");
				flow.update(1.0f / 60.0f, confirm);
				check(flow.getStageIndex() == stage + 1, "決定で次のステージへ進む");
			}
		}

		advance(flow, game::data::ENDING_DELAY + 3.0f);
		check(flow.getPhase() == game::flow::GamePhase::Ending, "最後のステージの後はエンディングになる");
		const game::flow::RestoreLevel& restore{ flow.getRestore() };
		check(restore.m_red > 0.99f && restore.m_blue > 0.99f && restore.m_yellowGreen > 0.99f, "エンディングではすべての色が戻っている");

		flow.update(1.0f / 60.0f, confirm);
		check(flow.getStageIndex() == 0 && flow.getPhase() == game::flow::GamePhase::StageIntro, "もういちど遊ぶとステージ 1 から始まる");
		check(flow.getRaisedPropCount(0) == 0 && flow.getRestore().m_red == 0.0f, "色と小物がリセットされる");
	}
} // namespace

int main()
{
	for (size_t i{}; i < game::data::STAGES.size(); ++i)
		testStage(i);
	testFlowStage1();
	testFullPlaythrough();

	std::printf("\n%s (%d 件失敗)\n", g_failCount == 0 ? "すべて成功" : "失敗あり", g_failCount);
	return g_failCount == 0 ? 0 : 1;
}
