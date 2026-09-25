#include "Assets.h"
#include "JpFont.h"
#include "infrastructure/ui/UiText.h"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>

namespace
{
	// 画像は tools/slice_sheet.py が art/ のシートから切り出したもの
	constexpr const char* IMAGE_DIR{ "resources/images/" };
	// ステージごとのタイルのテーマ(tiles/ の下のフォルダ名)。戻る色(赤・青・黄緑)に合わせている
	constexpr const char* STAGE_TILE_THEMES[]{ "meadow", "sea", "forest" };
	static_assert(std::size(STAGE_TILE_THEMES) == game::data::STAGES.size(), "ステージの数とテーマの数をそろえる");
	constexpr const char* UI_FONT_PATH{ "resources/fonts/NotoSansJP-Regular-subset.ttf" };
	constexpr int UI_FONT_SIZE{ 48 };    // UI の文字(一番大きく表示するサイズに合わせる)
	constexpr int TITLE_FONT_SIZE{ 96 }; // タイトルの文字

	/// 高 DPI の画面では、実際の画素数に合わせて大きく焼き込む(拡大されてぼやけるのを防ぐ)
	int scaledFontSize(int size)
	{
#if defined(PLATFORM_WEB)
		// Web 版は高 DPI 表示に未対応(キャンバスは 1280×720 のまま)なので、倍率を掛けない
		return size;
#else
		const Vector2 scale{ GetWindowScaleDPI() };
		return static_cast<int>(size * std::max(1.0f, scale.x));
#endif
	}

	// 何も読み込んでいないときに返す空のテクスチャ(描いても何も表示されない)
	const Texture2D EMPTY_TEXTURE{};

	// game::board::TileType の順
	constexpr const char* TILE_FILES[]{ "straight", "corner", "tee", "cross", "source", "goal", "locked", "blank" };
	// game::data::PropType の順
	constexpr const char* PROP_FILES[]{ "cottage", "flowers", "fence", "fountain", "lamp", "balloon", "tree", "pine" };
	// infrastructure::resource::BackgroundLayer の順
	constexpr const char* BACKGROUND_FILES[]{ "sky", "mountains", "village", "foreground" };

	/**
	 * @brief 画像を読み込み、拡大縮小・回転してもギザギザしないように設定する
	 * @param path ファイルのパス
	 * @param isOk 読み込めなかったら false にする
	 * @return テクスチャ
	 */
	Texture2D loadSmoothTexture(const std::string& path, bool& isOk)
	{
		Texture2D texture{ LoadTexture(path.c_str()) };
		if (!IsTextureValid(texture))
		{
			isOk = false;
			return texture;
		}
		// ミップマップ + 三線形補間: 縮小表示やカメラが動いたときに、細かい模様がちらつくのを抑える。
		// 【Web の落とし穴】WebGL1 は 2 のべき乗でない画像にミップマップを作れないので、その場合は双線形補間だけにする
		// (タイルは 128×128 で切り出しているので Web でもミップマップが効く)
		const bool isPowerOfTwo{ (texture.width & (texture.width - 1)) == 0 && (texture.height & (texture.height - 1)) == 0 };
#if defined(PLATFORM_WEB)
		const bool canUseMipmaps{ isPowerOfTwo };
#else
		const bool canUseMipmaps{ true };
		(void)isPowerOfTwo;
#endif
		if (canUseMipmaps)
		{
			GenTextureMipmaps(&texture);
			SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
		}
		else
		{
			SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
		}
		return texture;
	}
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
		m_uiFont = loadJapaneseFont(UI_FONT_PATH, scaledFontSize(UI_FONT_SIZE), usedText.c_str());
		m_titleFont = loadFontForText(UI_FONT_PATH, scaledFontSize(TITLE_FONT_SIZE), ui::TEXT_TITLE);

		bool isOk{ true };
		for (size_t stage{}; stage < m_tiles.size(); ++stage)
		{
			for (size_t type{}; type < m_tiles[stage].size(); ++type)
			{
				for (int variant{}; variant < TILE_VARIANT_COUNT; ++variant)
				{
					char fileName[32]{};
					std::snprintf(fileName, sizeof(fileName), "%s_%02d.png", TILE_FILES[type], variant);
					m_tiles[stage][type][variant] = loadSmoothTexture(std::string(IMAGE_DIR) + "tiles/" + STAGE_TILE_THEMES[stage] + "/" + fileName, isOk);
				}
			}
		}
		for (size_t i{}; i < m_backgrounds.size(); ++i)
			m_backgrounds[i] = loadSmoothTexture(std::string(IMAGE_DIR) + "bg/" + BACKGROUND_FILES[i] + ".png", isOk);
		for (size_t i{}; i < m_props.size(); ++i)
			m_props[i] = loadSmoothTexture(std::string(IMAGE_DIR) + "props/" + PROP_FILES[i] + ".png", isOk);

		// TODO: 主人公のコマとパレットを読み込む
		return isOk;
	}

	void Assets::unload()
	{
		for (auto& stage : m_tiles)
			for (auto& type : stage)
				for (Texture2D& texture : type)
					UnloadTexture(texture);
		for (Texture2D& texture : m_props)
			UnloadTexture(texture);
		for (Texture2D& texture : m_backgrounds)
			UnloadTexture(texture);
		m_tiles = {};
		m_props = {};
		m_backgrounds = {};
		UnloadFont(m_uiFont);
		m_uiFont = Font{};
		UnloadFont(m_titleFont);
		m_titleFont = Font{};
	}

	const Texture2D& Assets::getTileTexture(int stageIndex, game::board::TileType type, int variant) const
	{
		const int stage{ std::clamp(stageIndex, 0, static_cast<int>(m_tiles.size()) - 1) };
		const int wrapped{ ((variant % TILE_VARIANT_COUNT) + TILE_VARIANT_COUNT) % TILE_VARIANT_COUNT };
		return m_tiles[stage][static_cast<size_t>(type)][wrapped];
	}

	const Texture2D& Assets::getBackground(BackgroundLayer layer) const
	{
		return m_backgrounds[static_cast<size_t>(layer)];
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
