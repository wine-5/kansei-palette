#pragma once
#include "raylib.h"
#include "game/board/TileType.h"
#include "game/data/Stages.h"
#include "game/hero/HeroPose.h"
#include <array>
#include <vector>

namespace infrastructure::resource
{
	/**
	 * @brief 画像・フォントをまとめて読み込み、持っておく
	 * @details InitWindow() の後に load() を呼ぶ。音は infrastructure::audio::Audio が持つ
	 *          (ブラウザの自動再生制限のため、最初のクリック後に読み込む)。
	 */
	class Assets
	{
	public:
		/**
		 * @brief すべての画像とフォントを読み込む
		 * @return すべて読み込めたら true
		 */
		bool load();

		/// 読み込んだものをすべて解放する
		void unload();

		/**
		 * @brief タイルの画像を取得する
		 * @param type タイルの種類
		 * @return テクスチャ
		 */
		const Texture2D& getTileTexture(game::board::TileType type) const;

		/**
		 * @brief 小物の画像を取得する
		 * @param type 小物の種類
		 * @return テクスチャ
		 */
		const Texture2D& getPropTexture(game::data::PropType type) const;

		/**
		 * @brief 主人公のポーズのコマ数を取得する
		 * @param pose ポーズ
		 * @return コマ数(1 枚絵なら 1)
		 */
		int getHeroFrameCount(game::hero::HeroPose pose) const;

		/**
		 * @brief 主人公のポーズの 1 コマを取得する
		 * @param pose ポーズ
		 * @param frame コマ番号(0 〜 コマ数 - 1)
		 * @return テクスチャ
		 */
		const Texture2D& getHeroFrame(game::hero::HeroPose pose, int frame) const;

		/// タイトルのパレットの画像
		const Texture2D& getPaletteTexture() const { return m_palette; }

		/// UI 用のフォント
		const Font& getUiFont() const { return m_uiFont; }

	private:
		std::array<Texture2D, 8> m_tiles{};  // game::board::TileType の順
		std::array<Texture2D, 8> m_props{};  // game::data::PropType の順
		std::array<std::vector<Texture2D>, 7> m_heroFrames{}; // game::hero::HeroPose の順
		Texture2D m_palette{};
		Font m_uiFont{};
	};
} // namespace infrastructure::resource
