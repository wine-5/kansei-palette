#include "RestoreShader.h"
#include "rlgl.h"
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
uniform vec3 restore; // 赤, 青, 黄緑 の復元度(0〜1)

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

// 中心の色相からの距離が inner 以下なら 1、outer 以上なら 0、その間はなめらかに変化
float band(float h, float center, float inner, float outer)
{
    float d = abs(h - center);
    d = min(d, 360.0 - d);
    return 1.0 - smoothstep(inner, outer, d);
}

void main()
{
    vec4 texel = TEXTURE(texture0, fragTexCoord);
    vec4 tint = colDiffuse * fragColor;
    vec3 color = texel.rgb * tint.rgb;

    float h = hueOf(color); // 描画色で塗った単色の図形(DrawCube など)も色相で判定できるよう、tint を掛けた後の色で見る
    float k = band(h, 5.0, 22.0, 48.0) * restore.x
            + band(h, 232.0, 50.0, 78.0) * restore.y
            + band(h, 95.0, 45.0, 70.0) * restore.z;
    k = clamp(k, 0.0, 1.0);
    k = max(k, min(restore.x, min(restore.y, restore.z))); // 全色が戻ったら完全な彩色
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
		const float value[3]{ level.m_red, level.m_blue, level.m_yellowGreen };
		SetShaderValue(m_shader, m_restoreLocation, value, SHADER_UNIFORM_VEC3);
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
