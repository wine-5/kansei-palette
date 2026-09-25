#include "Particles.h"

namespace infrastructure::fx
{
	void Particles::spawnBurst(Vector3 position, int count, Color color)
	{
		// TODO: 実装する(m_nextIndex から count 粒を上書きし、ランダムな初速と寿命を与える)
		(void)position;
		(void)count;
		(void)color;
	}

	void Particles::update(float dt)
	{
		// TODO: 実装する(重力 PARTICLE_GRAVITY、地面で PARTICLE_BOUNCE 倍に減速して跳ね返る)
		(void)dt;
	}

	void Particles::draw(const Camera3D& camera) const
	{
		// TODO: 実装する(BeginBlendMode(BLEND_ADDITIVE) 〜 EndBlendMode で点を描く)
		(void)camera;
	}
} // namespace infrastructure::fx
