#pragma once
#include <array>

namespace game::data
{
	/**
	 * @brief 世界に戻る色 1 つ分
	 * @details ステージを 1 つクリアするごとに、色相の帯が 1 つずつ世界に戻る。
	 *          帯は色相環を 10 に分けたもので、隣の帯とは中間の色相で切り替わる。
	 */
	struct ColorBand
	{
		const char* m_name{};  // 画面に出す名前(「○○が もどった」)
		unsigned char m_red{}; // UI で使う代表の色
		unsigned char m_green{};
		unsigned char m_blue{};
		float m_hue{};         // 色相の中心(度)
	};

	/// 戻る色の数(= ステージ数)
	constexpr int COLOR_COUNT{ 10 };

	/// 戻る順に並べた色(色相環を一周する)
	inline constexpr std::array<ColorBand, COLOR_COUNT> COLOR_BANDS{ {
		{ "あか", 0xe5, 0x49, 0x3f, 5.0f },
		{ "だいだい", 0xf0, 0x8a, 0x2c, 30.0f },
		{ "きいろ", 0xf3, 0xc2, 0x1f, 50.0f },
		{ "きみどり", 0x8b, 0xc3, 0x4a, 80.0f },
		{ "みどり", 0x4c, 0xb8, 0x5a, 120.0f },
		{ "あおみどり", 0x26, 0xa6, 0x9a, 165.0f },
		{ "みずいろ", 0x2f, 0xb5, 0xc9, 192.0f },
		{ "あお", 0x3d, 0x8b, 0xea, 225.0f },
		{ "むらさき", 0x7b, 0x5c, 0xd6, 268.0f },
		{ "ももいろ", 0xe0, 0x5a, 0xa8, 320.0f },
	} };
} // namespace game::data
