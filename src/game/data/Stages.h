#pragma once
#include <array>

namespace game::data
{
	/**
	 * @brief ステージクリアで世界に戻る色の帯(要件定義 5章「色の復元」)
	 */
	enum class HueBand
	{
		Red,        // 赤(ステージ 1)
		Blue,       // 青(ステージ 2)
		YellowGreen // 黄・緑(ステージ 3)
	};

	/**
	 * @brief 箱庭の小物の種類(要件定義 7章「小物とパレット」)
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
		Pine
	};

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
	 */
	struct StageDefinition
	{
		const char* m_name{};   // 画面に出す名前
		const char* m_layout{}; // 盤面データ(game::board::parseStage の書式)
		HueBand m_hue = HueBand::Red;
		const char* m_hint{};   // 画面下のヒント文
		std::array<PropPlacement, MAX_PROPS_PER_STAGE> m_props{};
		int m_propCount{};
	};

	/// 全ステージ(要件定義 4章。初期状態・解答は tests/BoardTest.cpp で確認する)
	inline constexpr std::array<StageDefinition, 3> STAGES{ {
		{ "あか",
			"C2 B0 S1 B0 G2\n"
			"B0 T0 B0 C1 S1\n"
			"B0 S1 C0 S0 C1\n"
			"C3 B0 S1 B0 S0\n"
			"P3 S0 C0 B0 C2",
			HueBand::Red,
			"タイルをタップして回そう。でんきを ゴールまで とどける",
			{ { { PropType::Cottage, 1.9f, -3.9f, -2.1f, false },
				{ PropType::Flowers, 0.8f, 3.9f, -1.3f, false },
				{ PropType::Fence, 0.85f, 3.8f, 0.5f, false } } },
			3 },
		{ "あお",
			"C1 B0 S1 T2 C3\n"
			"G2 C0 B0 S1 B0\n"
			"C2 S0 T0 S0 G3\n"
			"B0 C2 L0 C0 S1\n"
			"S1 B0 P2 B0 C1",
			HueBand::Blue,
			"カギのタイルは回せない。ゴールは ふたつ",
			{ { { PropType::Fountain, 1.35f, 3.8f, 2.1f, false },
				{ PropType::Lamp, 1.9f, -4.1f, -0.2f, false },
				{ PropType::Balloon, 2.4f, 1.9f, -4.3f, false } } },
			3 },
		{ "きいろ・みどり",
			"P3 S0 C1 C2 B0\n"
			"C3 C0 X0 S0 C0\n"
			"B0 S1 L0 T1 S1\n"
			"G1 C2 S1 B0 G0\n"
			"S1 B0 G0 C0 B0",
			HueBand::YellowGreen,
			"十字のタイルから さんぼんに わかれる。ぜんぶ つなごう",
			{ { { PropType::Tree, 2.3f, 0.2f, -3.7f, false },
				{ PropType::Pine, 2.6f, -1.9f, -3.9f, false },
				{ PropType::Pine, 2.6f, 4.4f, -2.7f, false },
				{ PropType::Tree, 2.3f, -4.6f, 1.2f, true } } },
			4 },
	} };
} // namespace game::data
