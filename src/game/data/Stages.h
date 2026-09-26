#pragma once
#include "Colors.h"
#include <array>

namespace game::data
{
	/**
	 * @brief 箱庭の小物の種類
	 */
	enum class PropType
	{
		Cottage,
		Flowers,
		Fence,
		Fountain,
		Lamp,
		Balloon,
		Tree,
		Pine,
		Well,
		CrateBarrel,
		Signpost,
		Statue,
		Bridge,
		Hay,
		Mushroom
	};

	/// 小物の種類の数
	constexpr int PROP_TYPE_COUNT{ 15 };

	/**
	 * @brief 小物 1 つの配置
	 */
	struct PropPlacement
	{
		PropType m_type = PropType::Cottage;
		float m_height{}; // ゲーム内の高さ(ワールド単位)。幅は画像の縦横比で決まる
		float m_x{};      // 右が +
		float m_z{};      // 手前が +(盤面中心が 0, 0)
		bool m_isFlipped{}; // 左右反転するか
	};

	/// 1 ステージあたりの小物の最大数
	constexpr int MAX_PROPS_PER_STAGE{ 4 };

	/**
	 * @brief 1 ステージ分の定義
	 * @details ステージ番号 i をクリアすると、COLOR_BANDS[i] の色が世界に戻る(名前もその色の名前)。
	 */
	struct StageDefinition
	{
		const char* m_layout{}; // 盤面データ(game::board::parseStage の書式)
		const char* m_hint{};   // 画面下のヒント文
		std::array<PropPlacement, MAX_PROPS_PER_STAGE> m_props{};
		int m_propCount{};
	};

	/// 全ステージ(初期状態・解答は tests/BoardTest.cpp で確認する)
	inline constexpr std::array<StageDefinition, COLOR_COUNT> STAGES{ {
		// 1: あか
		{ "C2 B0 S1 B0 G2\n"
		  "B0 T0 B0 C1 S1\n"
		  "B0 S1 C0 S0 C1\n"
		  "C3 B0 S1 B0 S0\n"
		  "P3 S0 C0 B0 C2",
			"タイルをタップして回そう。でんきを ゴールまで とどける",
			{ { { PropType::Cottage, 1.9f, -3.9f, -2.1f, false }, { PropType::Flowers, 0.8f, 3.9f, -1.3f, false } } }, 2 },
		// 2: だいだい
		{ "C2 S2 S0 C2 C3\n"
		  "C1 S2 G3 C2 P1\n"
		  "S3 B3 C1 S2 S0\n"
		  "B1 C3 S3 C2 C3\n"
		  "B1 C1 S1 B2 B0",
			"まがりかどを うまく つなごう",
			{ { { PropType::Hay, 0.8f, 4.9f, 1.3f, false }, { PropType::Fence, 0.85f, 3.8f, 0.5f, false } } }, 2 },
		// 3: きいろ
		{ "P3 S1 C3 B1 C2\n"
		  "C1 C2 S1 S3 C3\n"
		  "L0 C0 C3 C2 S1\n"
		  "S1 S0 S0 S0 C1\n"
		  "G0 C2 C3 B0 S1",
			"カギのタイルは回せない",
			{ { { PropType::Lamp, 1.9f, -4.1f, -0.2f, false }, { PropType::CrateBarrel, 0.9f, 4.9f, -0.4f, false } } }, 2 },
		// 4: きみどり
		{ "C2 T1 C0 G2 C1\n"
		  "S1 P2 S3 L0 C1\n"
		  "G0 C3 C2 C1 S0\n"
		  "S2 C0 C2 C1 S1\n"
		  "C1 C3 S0 S1 S0",
			"T字で みちが ふたつに わかれる",
			{ { { PropType::Tree, 2.3f, 0.2f, -3.7f, false }, { PropType::Mushroom, 0.8f, -5.1f, -1.0f, false } } }, 2 },
		// 5: みどり
		{ "C1 B0 S1 T2 C3\n"
		  "G2 C0 B0 S1 B0\n"
		  "C2 S0 T0 S0 G3\n"
		  "B0 C2 L0 C0 S1\n"
		  "S1 B0 P2 B0 C1",
			"ゴールは ふたつ。ぜんぶ つなごう",
			{ { { PropType::Fountain, 1.35f, 3.8f, 2.1f, false }, { PropType::Well, 1.3f, 5.2f, -2.1f, false } } }, 2 },
		// 6: あおみどり
		{ "S3 S2 C1 G1 C0\n"
		  "C1 C1 S3 C3 C3\n"
		  "S0 S1 S0 T2 C0\n"
		  "S0 C0 C0 P2 L0\n"
		  "G1 S2 S0 S0 C2",
			"つかわない タイルも まざっている",
			{ { { PropType::Signpost, 1.1f, -3.2f, -3.2f, false }, { PropType::Pine, 2.6f, 4.4f, -2.7f, false } } }, 2 },
		// 7: みずいろ
		{ "S1 C3 S1 S1 B0\n"
		  "G1 L1 C3 S3 C1\n"
		  "G1 C1 L0 C2 C3\n"
		  "B3 S3 T2 C0 P0\n"
		  "C0 C0 C0 C3 C3",
			"カギの むきに あわせて みちを つくろう",
			{ { { PropType::Bridge, 0.9f, 3.3f, -4.0f, false } } }, 1 },
		// 8: あお
		{ "C2 T1 L1 C0 C2\n"
		  "S3 P2 S3 S3 T0\n"
		  "G0 C1 T2 C0 C1\n"
		  "C2 G0 C3 S0 G3\n"
		  "T2 C3 S3 C1 T3",
			"ゴールは みっつ",
			{ { { PropType::Balloon, 2.4f, 1.9f, -4.3f, false } } }, 1 },
		// 9: むらさき
		{ "P3 S0 C1 C2 B0\n"
		  "C3 C0 X0 S0 C0\n"
		  "B0 S1 L0 T1 S1\n"
		  "G1 C2 S1 B0 G0\n"
		  "S1 B0 G0 C0 B0",
			"十字のタイルから さんぼんに わかれる",
			{ { { PropType::Statue, 1.7f, -3.0f, -4.5f, false }, { PropType::Pine, 2.6f, -1.9f, -3.9f, false } } }, 2 },
		// 10: ももいろ
		{ "G1 S0 S2 T3 C0\n"
		  "C2 G2 C0 S1 S1\n"
		  "S0 T2 L1 C0 S3\n"
		  "S0 C3 L1 G3 P2\n"
		  "C0 S1 B1 B0 T0",
			"さいごの いろ。ぜんぶ つなげよう",
			{ { { PropType::Tree, 2.3f, -4.6f, 1.2f, true } } }, 1 },
	} };
} // namespace game::data
