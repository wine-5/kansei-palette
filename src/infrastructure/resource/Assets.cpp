#include "Assets.h"
#include "JpFont.h"
#include "infrastructure/ui/UiText.h"
#include <string>

namespace
{
	// TODO: 要件定義 7章は assets/、テンプレートは resources/。どちらかに統一する
	constexpr const char* ASSET_DIR{ "resources/" };
	constexpr const char* UI_FONT_PATH{ "resources/fonts/NotoSansJP-Regular-subset.ttf" };
	constexpr int UI_FONT_SIZE{ 48 };

	// 何も読み込んでいないときに返す空のテクスチャ(描いても何も表示されない)
	const Texture2D EMPTY_TEXTURE{};
} // namespace

namespace infrastructure::resource
{
	bool Assets::load()
	{
		// フォントに焼き込む文字 = UI の文章 + 全ステージの名前とヒント文
		std::string usedText{ ui::TEXT_ALL_UI };
		for (const auto& stage : game::data::STAGES)
		{
			usedText += stage.m_name;
			usedText += stage.m_hint;
		}
		m_uiFont = loadJapaneseFont(UI_FONT_PATH, UI_FONT_SIZE, usedText.c_str());

		// TODO: 実装する(タイル 8 枚、小物 8 枚、主人公のコマ、パレットを ASSET_DIR から読み込む)
		(void)ASSET_DIR;
		return true;
	}

	void Assets::unload()
	{
		// TODO: 実装する(読み込んだテクスチャをすべて UnloadTexture する)
		UnloadFont(m_uiFont);
		m_uiFont = Font{};
	}

	const Texture2D& Assets::getTileTexture(game::board::TileType type) const
	{
		return m_tiles[static_cast<size_t>(type)];
	}

	const Texture2D& Assets::getPropTexture(game::data::PropType type) const
	{
		return m_props[static_cast<size_t>(type)];
	}

	int Assets::getHeroFrameCount(game::hero::HeroPose pose) const
	{
		return static_cast<int>(m_heroFrames[static_cast<size_t>(pose)].size());
	}

	const Texture2D& Assets::getHeroFrame(game::hero::HeroPose pose, int frame) const
	{
		const auto& frames{ m_heroFrames[static_cast<size_t>(pose)] };
		if (frame < 0 || frame >= static_cast<int>(frames.size()))
			return EMPTY_TEXTURE;
		return frames[frame];
	}
} // namespace infrastructure::resource
