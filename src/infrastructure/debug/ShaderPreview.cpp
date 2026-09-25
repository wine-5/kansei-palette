#include "ShaderPreview.h"
#include "infrastructure/render/RestoreShader.h"

namespace
{
	constexpr int STRIP_WIDTH{ 360 }; // 1px = 色相 1 度
	constexpr int STRIP_HEIGHT{ 40 };

	/**
	 * @brief 確認する段(ラベルと復元度)
	 */
	struct PreviewRow
	{
		const char* m_label{};
		game::flow::RestoreLevel m_level{};
		float m_selfRestore{}; // tint で渡す「通電度」
	};

	const PreviewRow ROWS[]{
		{ "original (no shader)", {}, 0.0f },
		{ "restore: none", { 0.0f, 0.0f, 0.0f }, 0.0f },
		{ "restore: red", { 1.0f, 0.0f, 0.0f }, 0.0f },
		{ "restore: red + blue", { 1.0f, 1.0f, 0.0f }, 0.0f },
		{ "restore: all", { 1.0f, 1.0f, 1.0f }, 0.0f },
		{ "none + tile powered 0.5", { 0.0f, 0.0f, 0.0f }, 0.5f },
		{ "none + tile powered 1.0", { 0.0f, 0.0f, 0.0f }, 1.0f },
	};
} // namespace

namespace infrastructure::debug
{
	void ShaderPreview::init()
	{
		// 上半分は鮮やかな色、下半分は少し暗く彩度の低い色
		Image image{ GenImageColor(STRIP_WIDTH, STRIP_HEIGHT, BLANK) };
		for (int x{}; x < STRIP_WIDTH; ++x)
		{
			const float hue{ static_cast<float>(x) };
			for (int y{}; y < STRIP_HEIGHT; ++y)
			{
				const bool isTop{ y < STRIP_HEIGHT / 2 };
				ImageDrawPixel(&image, x, y, ColorFromHSV(hue, isTop ? 0.9f : 0.5f, isTop ? 1.0f : 0.7f));
			}
		}
		m_hueStrip = LoadTextureFromImage(image);
		UnloadImage(image);
	}

	void ShaderPreview::unload()
	{
		UnloadTexture(m_hueStrip);
	}

	void ShaderPreview::update()
	{
		if (IsKeyPressed(KEY_F1))
			m_isVisible = !m_isVisible;
		if (IsKeyPressed(KEY_F3))
			m_isForcingFullRestore = !m_isForcingFullRestore;
	}

	void ShaderPreview::draw(render::RestoreShader& shader) const
	{
		if (!m_isVisible)
			return;

		constexpr int left{ 40 };
		constexpr int top{ 40 };
		constexpr int rowHeight{ STRIP_HEIGHT + 24 };
		constexpr float scaleX{ 2.0f };
		const int rowCount{ static_cast<int>(sizeof(ROWS) / sizeof(ROWS[0])) };

		DrawRectangle(left - 16, top - 16, static_cast<int>(STRIP_WIDTH * scaleX) + 32, rowHeight * rowCount + 40, Fade(BLACK, 0.85f));
		DrawText(shader.isLoaded() ? "RestoreShader: loaded (F1 to close)" : "RestoreShader: NOT LOADED", left, top - 8, 16, shader.isLoaded() ? GREEN : RED);

		for (int i{}; i < rowCount; ++i)
		{
			const PreviewRow& row{ ROWS[i] };
			const int y{ top + 16 + i * rowHeight };
			DrawText(row.m_label, left, y, 14, RAYWHITE);

			const Rectangle src{ 0.0f, 0.0f, static_cast<float>(STRIP_WIDTH), static_cast<float>(STRIP_HEIGHT) };
			const Rectangle dst{ static_cast<float>(left), static_cast<float>(y + 16), STRIP_WIDTH * scaleX, static_cast<float>(STRIP_HEIGHT) };
			if (i == 0)
			{
				DrawTexturePro(m_hueStrip, src, dst, Vector2{}, 0.0f, WHITE);
				continue;
			}

			// uniform を変える前に、前の段の描画を end() で GPU に送り切っておく
			shader.setRestoreLevel(row.m_level);
			shader.begin();
			DrawTexturePro(m_hueStrip, src, dst, Vector2{}, 0.0f, render::RestoreShader::makeTint(WHITE, row.m_selfRestore));
			shader.end();
		}
	}
} // namespace infrastructure::debug
