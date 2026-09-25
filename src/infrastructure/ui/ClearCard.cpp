#include "ClearCard.h"
#include "infrastructure/resource/Assets.h"

namespace infrastructure::ui
{
	UiAction ClearCard::update(const game::flow::GameFlow& flow)
	{
		// TODO: 実装する(「つぎへ」ボタンの判定)
		(void)flow;
		return UiAction{};
	}

	void ClearCard::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		// TODO: 実装する
		(void)flow;
		(void)assets;
	}
} // namespace infrastructure::ui
