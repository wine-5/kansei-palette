#include "RestoreShader.h"

namespace infrastructure::render
{
	bool RestoreShader::load()
	{
		// TODO: 実装する
		// - PLATFORM_WEB なら "#version 100\nprecision mediump float;\n"、それ以外は "#version 330\n" を先頭に付ける
		// - フラグメントシェーダーで色相 → band() → mix(gray, 元の色, k)
		// - k には tint のアルファ(タイルの通電度)も反映する
		m_isLoaded = false;
		return m_isLoaded;
	}

	void RestoreShader::unload()
	{
		if (m_isLoaded)
			UnloadShader(m_shader);
		m_isLoaded = false;
	}

	void RestoreShader::setRestoreLevel(const game::flow::RestoreLevel& level)
	{
		// TODO: 実装する(SetShaderValue で vec3 を渡す)
		(void)level;
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
} // namespace infrastructure::render
