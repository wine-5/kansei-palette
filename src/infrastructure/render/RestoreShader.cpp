#include "RestoreShader.h"
#include "rlgl.h"
#include <cmath>
#include <string>

namespace
{
	// プラットフォームごとの違い(バージョン・入出力の書き方)だけをここで吸収する。
	// GLSL の #define は C++ のマクロ禁止規約の対象外(シェーダー側の書き方の差を埋めるため)
#if defined(PLATFORM_WEB)
	constexpr const char* SHADER_HEADER{
		"#version 100\n"
		"precision mediump float;\n"
		"#define IN varying\n"
		"#define TEXTURE texture2D\n"
		"#define OUT_COLOR gl_FragColor\n"
	};
#else
	constexpr const char* SHADER_HEADER{
		"#version 330\n"
		"#define IN in\n"
		"#define TEXTURE texture\n"
		"out vec4 finalColor;\n"
		"#define OUT_COLOR finalColor\n"
	};
#endif

	// 色の復元。頂点シェーダーは raylib の標準のものを使う
	constexpr const char* SHADER_BODY{ R"(
IN vec2 fragTexCoord;
IN vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
// 色相の帯(game::data::COLOR_BANDS の順)。隣の帯とは中間の色相で切り替わる
uniform float bandCenter[10];   // 帯の中心の色相(度)
uniform float bandGapLeft[10];  // 色相が小さい側の隣の帯までの距離(度)
uniform float bandGapRight[10]; // 色相が大きい側の隣の帯までの距離(度)
uniform float restore[10];      // 帯ごとの復元度(0〜1)

// RGB → 色相(0〜360 度)
float hueOf(vec3 c)
{
    float maxC = max(c.r, max(c.g, c.b));
    float minC = min(c.r, min(c.g, c.b));
    float d = maxC - minC;
    if (d < 0.0001) return 0.0;
    float h;
    if (maxC == c.r) h = mod((c.g - c.b) / d, 6.0);
    else if (maxC == c.g) h = (c.b - c.r) / d + 2.0;
    else h = (c.r - c.g) / d + 4.0;
    return h * 60.0;
}

// 色相 h が帯にどれだけ含まれるか。隣の帯との中間までは 1、隣の帯の中心で 0 になる
float band(float h, float center, float gapLeft, float gapRight)
{
    float d = h - center;
    if (d > 180.0) d -= 360.0;
    if (d < -180.0) d += 360.0;
    float gap = d < 0.0 ? gapLeft : gapRight;
    return 1.0 - smoothstep(gap * 0.5, gap, abs(d));
}

void main()
{
    vec4 texel = TEXTURE(texture0, fragTexCoord);
    vec4 tint = colDiffuse * fragColor;
    vec3 color = texel.rgb * tint.rgb;

    float h = hueOf(color); // 描画色で塗った単色の図形(DrawCube など)も色相で判定できるよう、tint を掛けた後の色で見る
    float k = 0.0;
    float allRestored = 1.0;
    for (int i = 0; i < 10; i++)
    {
        k += band(h, bandCenter[i], bandGapLeft[i], bandGapRight[i]) * restore[i];
        allRestored = min(allRestored, restore[i]);
    }
    k = clamp(k, 0.0, 1.0);
    k = max(k, allRestored); // 全色が戻ったら完全な彩色(色の薄い灰色なども含めて)
    k = max(k, 1.0 - tint.a);                              // tint のアルファ = 世界に従う度合い

    float luma = dot(color, vec3(0.299, 0.587, 0.114));
    float gray = (luma - 0.5) * 1.06 + 0.5;
    OUT_COLOR = vec4(mix(vec3(gray), color, k), texel.a);
}
)" };
} // namespace

namespace infrastructure::render
{
	bool RestoreShader::load()
	{
		const std::string fragmentCode{ std::string(SHADER_HEADER) + SHADER_BODY };
		m_shader = LoadShaderFromMemory(nullptr, fragmentCode.c_str());
		m_isLoaded = IsShaderValid(m_shader) && m_shader.id != rlGetShaderIdDefault();
		if (!m_isLoaded)
		{
			TraceLog(LOG_WARNING, "RestoreShader: シェーダーを読み込めませんでした(色の復元なしで描画します)");
			return false;
		}

		m_restoreLocation = GetShaderLocation(m_shader, "restore");

		// 帯の中心と、左右の隣の帯までの距離を渡す(色相環なので 0 度と 360 度はつながっている)
		constexpr int count{ game::data::COLOR_COUNT };
		float centers[count]{};
		float gapsLeft[count]{};
		float gapsRight[count]{};
		for (int i{}; i < count; ++i)
		{
			const float center{ game::data::COLOR_BANDS[i].m_hue };
			const float previous{ game::data::COLOR_BANDS[(i + count - 1) % count].m_hue };
			const float next{ game::data::COLOR_BANDS[(i + 1) % count].m_hue };
			centers[i] = center;
			gapsLeft[i] = std::fmod(center - previous + 360.0f, 360.0f);
			gapsRight[i] = std::fmod(next - center + 360.0f, 360.0f);
		}
		SetShaderValueV(m_shader, GetShaderLocation(m_shader, "bandCenter"), centers, SHADER_UNIFORM_FLOAT, count);
		SetShaderValueV(m_shader, GetShaderLocation(m_shader, "bandGapLeft"), gapsLeft, SHADER_UNIFORM_FLOAT, count);
		SetShaderValueV(m_shader, GetShaderLocation(m_shader, "bandGapRight"), gapsRight, SHADER_UNIFORM_FLOAT, count);
		setRestoreLevel(game::flow::RestoreLevel{});
		return true;
	}

	void RestoreShader::unload()
	{
		if (m_isLoaded)
			UnloadShader(m_shader);
		m_isLoaded = false;
	}

	void RestoreShader::setRestoreLevel(const game::flow::RestoreLevel& level)
	{
		if (!m_isLoaded)
			return;
		SetShaderValueV(m_shader, m_restoreLocation, level.m_levels.data(), SHADER_UNIFORM_FLOAT, static_cast<int>(level.m_levels.size()));
	}

	void RestoreShader::begin() const
	{
		if (m_isLoaded)
			BeginShaderMode(m_shader);
	}

	void RestoreShader::end() const
	{
		if (m_isLoaded)
			EndShaderMode();
	}

	Color RestoreShader::makeTint(Color base, float selfRestore)
	{
		const float clamped{ selfRestore < 0.0f ? 0.0f : (selfRestore > 1.0f ? 1.0f : selfRestore) };
		base.a = static_cast<unsigned char>((1.0f - clamped) * 255.0f + 0.5f);
		return base;
	}
} // namespace infrastructure::render
