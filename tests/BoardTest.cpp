// 盤面ロジックのテスト(raylib なしで動く)。
// 実行: cmake --build build --target KanseiTests && build\Debug\KanseiTests.exe
// 確認すること(要件定義 FR-04, FR-06):
//   - 3 ステージとも、初期状態ではクリアしていない
//   - 解答表どおりに回すとクリアできる
#include "game/board/StageParser.h"
#include "game/data/Stages.h"
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

	/// 要件定義 4章の解答表(十字タイルは向きが任意なので含めない)
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

		// 目標の回転数になるまで回す
		for (const auto& step : SOLUTIONS[stageIndex])
		{
			for (int i{}; i < 4 && board->getTile(step.m_row, step.m_col).m_rotation != step.m_rotation; ++i)
				board->rotate(step.m_row, step.m_col);
		}
		check(board->isCleared(), "解答どおりに回すとクリアできる");
	}
} // namespace

int main()
{
	for (size_t i{}; i < game::data::STAGES.size(); ++i)
		testStage(i);

	std::printf("\n%s (%d 件失敗)\n", g_failCount == 0 ? "すべて成功" : "失敗あり", g_failCount);
	return g_failCount == 0 ? 0 : 1;
}
