#include "Effects.h"

namespace infrastructure::fx
{
	void Effects::onEvents(const game::event::GameEventList& events, const game::flow::GameFlow& flow)
	{
		// TODO: 実装する
		// - StageCleared: フラッシュ(0.55)、カメラの揺れ(0.32)、各ゴールから星くず 70 粒、輪
		// - PropRaised: その小物の位置から星くず 26 粒
		// - EndingStarted: 強いフラッシュ、色とりどりの星くずを降らせ続ける
		(void)events;
		(void)flow;
	}

	void Effects::update(float dt)
	{
		// TODO: 実装する(フラッシュは 0.9 秒で消す、揺れは毎秒 6 で減衰)
		m_particles.update(dt);
	}

	void Effects::drawWorld(const Camera3D& camera) const
	{
		m_particles.draw(camera);
	}

	void Effects::drawScreen() const
	{
		// TODO: 実装する(m_flashAlpha で画面全体を白く覆う)
	}
} // namespace infrastructure::fx
