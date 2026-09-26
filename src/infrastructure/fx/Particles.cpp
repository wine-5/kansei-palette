#include "Particles.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>

namespace
{
	constexpr int TEXTURE_SIZE{ 32 };
	constexpr float LIFE_MIN{ 0.9f };
	constexpr float LIFE_MAX{ 1.9f };
	// SIZE_MAX は標準ライブラリのマクロと衝突するので、PARTICLE_ を付ける
	constexpr float PARTICLE_SIZE_MIN{ 0.16f };
	constexpr float PARTICLE_SIZE_MAX{ 0.34f };
	constexpr float GROUND_Y{ 0.03f };        // 盤面の高さ(ここで跳ね返る)
	constexpr float GROUND_FRICTION{ 0.8f };  // 跳ね返ったときの横の減速
	constexpr float FALLING_LIFE{ 3.0f };
	constexpr float FALLING_GRAVITY{ 0.6f };  // 降らせる粒はふわっと落とす

	/// min〜max の乱数
	float randomRange(float min, float max)
	{
		return min + (max - min) * (GetRandomValue(0, 10000) / 10000.0f);
	}
} // namespace

namespace infrastructure::fx
{
	void Particles::init()
	{
		Image image{ GenImageGradientRadial(TEXTURE_SIZE, TEXTURE_SIZE, 0.0f, WHITE, BLANK) };
		m_texture = LoadTextureFromImage(image);
		UnloadImage(image);
		SetTextureFilter(m_texture, TEXTURE_FILTER_BILINEAR);
	}

	void Particles::unload()
	{
		UnloadTexture(m_texture);
		m_texture = Texture2D{};
	}

	Particles::Particle& Particles::nextParticle()
	{
		Particle& particle{ m_particles[m_nextIndex] };
		m_nextIndex = (m_nextIndex + 1) % static_cast<int>(m_particles.size());
		return particle;
	}

	void Particles::spawnBurst(Vector3 position, int count, const Color* colors, int colorCount, float speed)
	{
		for (int i{}; i < count; ++i)
		{
			Particle& particle{ nextParticle() };
			const float angle{ randomRange(0.0f, 2.0f * PI) };
			const float horizontal{ randomRange(0.8f, 2.6f) * speed };
			particle.m_position = position;
			particle.m_velocity = Vector3{ std::cos(angle) * horizontal, randomRange(2.5f, 5.5f) * speed, std::sin(angle) * horizontal };
			particle.m_color = colors[GetRandomValue(0, colorCount - 1)];
			particle.m_maxLife = randomRange(LIFE_MIN, LIFE_MAX);
			particle.m_life = particle.m_maxLife;
			particle.m_size = randomRange(PARTICLE_SIZE_MIN, PARTICLE_SIZE_MAX);
			particle.m_gravity = game::data::PARTICLE_GRAVITY;
		}
	}

	void Particles::spawnFalling(Vector3 position, Color color)
	{
		Particle& particle{ nextParticle() };
		particle.m_position = position;
		particle.m_velocity = Vector3{ randomRange(-0.3f, 0.3f), randomRange(-0.8f, -0.3f), randomRange(-0.3f, 0.3f) };
		particle.m_color = color;
		particle.m_maxLife = FALLING_LIFE;
		particle.m_life = FALLING_LIFE;
		particle.m_size = randomRange(PARTICLE_SIZE_MIN, PARTICLE_SIZE_MAX);
		particle.m_gravity = FALLING_GRAVITY;
	}

	void Particles::update(float dt)
	{
		for (Particle& particle : m_particles)
		{
			if (particle.m_life <= 0.0f)
				continue;
			particle.m_life -= dt;
			particle.m_velocity.y -= particle.m_gravity * dt;
			particle.m_position.x += particle.m_velocity.x * dt;
			particle.m_position.y += particle.m_velocity.y * dt;
			particle.m_position.z += particle.m_velocity.z * dt;

			// 地面で跳ね返る(速さは PARTICLE_BOUNCE 倍に落ちる)
			if (particle.m_position.y < GROUND_Y && particle.m_velocity.y < 0.0f)
			{
				particle.m_position.y = GROUND_Y;
				particle.m_velocity.y = -particle.m_velocity.y * game::data::PARTICLE_BOUNCE;
				particle.m_velocity.x *= GROUND_FRICTION;
				particle.m_velocity.z *= GROUND_FRICTION;
			}
		}
	}

	void Particles::draw(const Camera3D& camera) const
	{
		// 光の粒は重なるほど明るくなる加算合成。奥のものを隠さないよう、深度は書き込まない
		rlDisableDepthMask();
		BeginBlendMode(BLEND_ADDITIVE);
		for (const Particle& particle : m_particles)
		{
			if (particle.m_life <= 0.0f)
				continue;
			// 寿命の終わりに向けて小さく、薄くする
			const float t{ particle.m_life / particle.m_maxLife };
			Color color{ particle.m_color };
			color.a = static_cast<unsigned char>(255.0f * std::min(1.0f, t * 1.5f));
			DrawBillboard(camera, m_texture, particle.m_position, particle.m_size * (0.4f + 0.6f * t), color);
		}
		EndBlendMode();
		rlEnableDepthMask();
	}
} // namespace infrastructure::fx
