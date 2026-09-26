#include "Effects.h"
#include "core/Easing.h"
#include "game/data/Stages.h"
#include "infrastructure/render/WorldRenderer.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>
#include <iterator>

namespace
{
	// クリアの瞬間
	constexpr float CLEAR_FLASH{ 0.55f };        // 白いフラッシュの不透明度
	constexpr float FLASH_TIME{ 0.9f };          // フラッシュが消えるまでの秒数
	constexpr float CLEAR_SHAKE{ 0.32f };        // カメラの揺れの強さ
	constexpr float SHAKE_DECAY{ 6.0f };         // 揺れの減衰(毎秒)
	constexpr float SHAKE_AMPLITUDE{ 0.35f };    // 揺れの強さ 1.0 あたりのずれ(ワールド単位)
	constexpr int GOAL_BURST{ 70 };              // ゴールごとの星くずの数
	constexpr float GOAL_RING_RADIUS{ 3.4f };    // ゴールから広がる輪
	constexpr float BOARD_RING_RADIUS{ 7.5f };   // 盤面全体に広がる輪
	constexpr float RING_TIME{ 1.1f };           // 輪が広がりきって消えるまでの秒数
	constexpr int RING_TEXTURE_SIZE{ 128 };
	constexpr float RING_TEXTURE_EDGE{ 0.88f };  // 輪の明るい部分の位置(半径に対する割合)
	constexpr float RING_TEXTURE_WIDTH{ 0.07f }; // 輪の太さ(半径に対する割合)

	// 小物がせり上がったとき
	constexpr int PROP_BURST{ 26 };

	// エンディング
	constexpr float ENDING_FLASH{ 0.9f };
	constexpr float RAIN_PER_SECOND{ 45.0f };    // 降らせる星くずの数(毎秒)
	constexpr float RAIN_RADIUS{ 5.5f };         // 降らせる範囲(島の中心からの距離)
	constexpr float RAIN_HEIGHT{ 4.5f };


	/// エンディングの色とりどりの星くず
	constexpr Color RAINBOW_COLORS[]{
		{ 0xe5, 0x49, 0x3f, 255 }, { 0xf0, 0x8a, 0x2c, 255 }, { 0xf3, 0xc2, 0x1f, 255 }, { 0x8b, 0xc3, 0x4a, 255 },
		{ 0x2f, 0xb5, 0xc9, 255 }, { 0x3d, 0x8b, 0xea, 255 }, { 0x7b, 0x5c, 0xd6, 255 }, { 0xc2, 0x57, 0xc4, 255 },
	};

	float randomRange(float min, float max)
	{
		return min + (max - min) * (GetRandomValue(0, 10000) / 10000.0f);
	}

	/// ステージの星くずの色: 戻る色と、それを白に近づけた 3 色
	std::array<Color, 4> stageColors(int stageIndex)
	{
		const game::data::ColorBand& band{ game::data::COLOR_BANDS[stageIndex % game::data::COLOR_COUNT] };
		std::array<Color, 4> colors{};
		for (int i{}; i < 4; ++i)
		{
			const float t{ i * 0.28f }; // 0, 0.28, 0.56, 0.84 だけ白に寄せる
			colors[i] = Color{ static_cast<unsigned char>(band.m_red + (255 - band.m_red) * t), static_cast<unsigned char>(band.m_green + (255 - band.m_green) * t),
				static_cast<unsigned char>(band.m_blue + (255 - band.m_blue) * t), 255 };
		}
		return colors;
	}
} // namespace

namespace infrastructure::fx
{
	void Effects::init()
	{
		m_particles.init();

		// 輪の画像: 半径の RING_TEXTURE_EDGE の位置だけが明るく、内外へなめらかに消える
		Image image{ GenImageColor(RING_TEXTURE_SIZE, RING_TEXTURE_SIZE, BLANK) };
		const float half{ RING_TEXTURE_SIZE / 2.0f };
		for (int y{}; y < RING_TEXTURE_SIZE; ++y)
		{
			for (int x{}; x < RING_TEXTURE_SIZE; ++x)
			{
				const float r{ std::hypot(x + 0.5f - half, y + 0.5f - half) / half };
				const float d{ (r - RING_TEXTURE_EDGE) / RING_TEXTURE_WIDTH };
				const float alpha{ std::exp(-d * d) };
				ImageDrawPixel(&image, x, y, Color{ 255, 255, 255, static_cast<unsigned char>(255.0f * alpha) });
			}
		}
		m_ringTexture = LoadTextureFromImage(image);
		UnloadImage(image);
		SetTextureFilter(m_ringTexture, TEXTURE_FILTER_BILINEAR);
		m_ringModel = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
		m_ringModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = m_ringTexture;
	}

	void Effects::unload()
	{
		m_particles.unload();
		// UnloadModel が借りている画像まで解放しないよう、既定に戻してから解放する
		m_ringModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
		UnloadModel(m_ringModel);
		m_ringModel = Model{};
		UnloadTexture(m_ringTexture);
		m_ringTexture = Texture2D{};
	}

	void Effects::onEvents(const game::event::GameEventList& events, const game::flow::GameFlow& flow)
	{
		const int stageIndex{ flow.getStageIndex() };
		for (const auto& event : events)
		{
			switch (event.m_type)
			{
			case game::event::GameEventType::StageCleared:
			{
				// 各ゴールから星くずを打ち上げ、輪を広げる。盤面全体にも大きな輪
				const std::array<Color, 4> colors{ stageColors(stageIndex) };
				const game::board::Board& board{ flow.getBoard() };
				for (int row{}; row < game::board::Board::SIZE; ++row)
				{
					for (int col{}; col < game::board::Board::SIZE; ++col)
					{
						if (board.getTile(row, col).m_type != game::board::TileType::Goal)
							continue;
						const Vector3 position{ render::WorldRenderer::cellToWorld(row, col) };
						m_particles.spawnBurst(position, GOAL_BURST, colors.data(), static_cast<int>(colors.size()));
						spawnRing(position, GOAL_RING_RADIUS, colors[0]);
					}
				}
				spawnRing(Vector3{}, BOARD_RING_RADIUS, colors[1]);
				startFlash(CLEAR_FLASH);
				startShake(CLEAR_SHAKE);
				break;
			}
			case game::event::GameEventType::PropRaised:
			{
				// せり上がった小物の足元から星くず(m_value = 小物の番号)
				const game::data::PropPlacement& prop{ game::data::STAGES[stageIndex].m_props[event.m_value] };
				const std::array<Color, 4> colors{ stageColors(stageIndex) };
				m_particles.spawnBurst(Vector3{ prop.m_x, prop.m_height * 0.3f, prop.m_z }, PROP_BURST, colors.data(), static_cast<int>(colors.size()), 0.7f);
				break;
			}
			case game::event::GameEventType::EndingStarted:
				startFlash(ENDING_FLASH);
				m_isEndingRain = true;
				break;
			case game::event::GameEventType::StageStarted:
			case game::event::GameEventType::GameStarted: m_isEndingRain = false; break;
			default: break;
			}
		}
	}

	void Effects::update(float dt)
	{
		m_particles.update(dt);

		for (Ring& ring : m_rings)
		{
			if (!ring.m_isActive)
				continue;
			ring.m_time += dt;
			ring.m_isActive = ring.m_time < RING_TIME;
		}

		// フラッシュは FLASH_TIME 秒でまっすぐ消す
		m_flashAlpha = std::max(0.0f, m_flashAlpha - m_flashStart / FLASH_TIME * dt);

		// 揺れは速い波で前後左右に振り、強さを指数的に弱める
		m_shakeTime += dt;
		m_shakeStrength = core::approachExp(m_shakeStrength, 0.0f, SHAKE_DECAY, dt);
		const float amplitude{ m_shakeStrength * SHAKE_AMPLITUDE };
		m_cameraShake = Vector3{ std::sin(m_shakeTime * 47.0f) * amplitude, std::sin(m_shakeTime * 61.0f + 1.3f) * amplitude, 0.0f };

		// エンディング中は、島の上に色とりどりの星くずを降らせ続ける
		if (m_isEndingRain)
		{
			m_rainTimer += dt * RAIN_PER_SECOND;
			while (m_rainTimer >= 1.0f)
			{
				m_rainTimer -= 1.0f;
				const float angle{ randomRange(0.0f, 2.0f * PI) };
				const float radius{ RAIN_RADIUS * std::sqrt(randomRange(0.0f, 1.0f)) };
				const Vector3 position{ std::cos(angle) * radius, RAIN_HEIGHT + randomRange(0.0f, 1.0f), std::sin(angle) * radius };
				m_particles.spawnFalling(position, RAINBOW_COLORS[GetRandomValue(0, static_cast<int>(std::size(RAINBOW_COLORS)) - 1)]);
			}
		}
	}

	void Effects::drawWorld(const Camera3D& camera) const
	{
		// 輪: 地面に寝かせた輪の画像を、広げながら薄くする(加算合成、深度は書き込まない)
		rlDisableDepthMask();
		BeginBlendMode(BLEND_ADDITIVE);
		for (const Ring& ring : m_rings)
		{
			if (!ring.m_isActive)
				continue;
			const float t{ ring.m_time / RING_TIME };
			const float diameter{ std::max(0.01f, ring.m_maxRadius * core::easeOutCubic(t)) * 2.0f };
			Color color{ ring.m_color };
			color.a = static_cast<unsigned char>(255.0f * (1.0f - t));
			const Vector3 center{ ring.m_center.x, 0.07f, ring.m_center.z };
			DrawModelEx(m_ringModel, center, Vector3{ 0.0f, 1.0f, 0.0f }, 0.0f, Vector3{ diameter, 1.0f, diameter }, color);
		}
		EndBlendMode();
		rlEnableDepthMask();

		m_particles.draw(camera);
	}

	void Effects::drawScreen() const
	{
		if (m_flashAlpha <= 0.0f)
			return;
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, m_flashAlpha));
	}

	void Effects::spawnRing(Vector3 center, float maxRadius, Color color)
	{
		m_rings[m_nextRing] = Ring{ center, maxRadius, 0.0f, color, true };
		m_nextRing = (m_nextRing + 1) % static_cast<int>(m_rings.size());
	}

	void Effects::startFlash(float alpha)
	{
		m_flashAlpha = alpha;
		m_flashStart = alpha;
	}

	void Effects::startShake(float strength)
	{
		m_shakeStrength = strength;
		m_shakeTime = 0.0f;
	}
} // namespace infrastructure::fx
