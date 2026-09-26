#pragma once
#include "game/data/Colors.h"
#include <array>

namespace game::flow
{
	/**
	 * @brief 世界に戻った色の度合い(0〜1)。game::data::COLOR_BANDS の順
	 * @details 色の復元シェーダーの uniform にそのまま渡す。
	 */
	struct RestoreLevel
	{
		std::array<float, data::COLOR_COUNT> m_levels{};

		/// すべての色が戻った状態
		static RestoreLevel full()
		{
			RestoreLevel level;
			level.m_levels.fill(1.0f);
			return level;
		}

		/// 戻った(半分以上)色の数
		int countRestored() const
		{
			int count{};
			for (float level : m_levels)
			{
				if (level > 0.5f)
					++count;
			}
			return count;
		}
	};
} // namespace game::flow
