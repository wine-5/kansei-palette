#pragma once

namespace game::flow
{
	/**
	 * @brief 世界に戻った色の度合い(0〜1)
	 * @details 色の復元シェーダーの uniform(r, b, g)にそのまま渡す(要件定義 5章「色の復元」)。
	 */
	struct RestoreLevel
	{
		float m_red{};
		float m_blue{};
		float m_yellowGreen{};
	};
} // namespace game::flow
