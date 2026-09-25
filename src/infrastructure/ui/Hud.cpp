#include "Hud.h"
#include "infrastructure/resource/Assets.h"

namespace infrastructure::ui
{
	UiAction Hud::update(const game::flow::GameFlow& flow)
	{
		// TODO: 実装する(「やりなおす」「音」ボタンの判定)
		(void)flow;
		return UiAction{};
	}

	void Hud::draw(const game::flow::GameFlow& flow, const resource::Assets& assets) const
	{
		// TODO: 実装する
		(void)flow;
		(void)assets;
	}
} // namespace infrastructure::ui
